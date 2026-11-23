/*  ClientSocket Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Loopback tests that drive the "ClientSocket" backends against a
 *  "ServerSocket" peer on 127.0.0.1. These are the client-side
 *  behaviors of the old ClientSocket/ServerSocket pair that the
 *  "ServerSocket" test suite (which covers the server side of the same
 *  scenarios) does not exercise: the send deadline, a pre-close
 *  response while the client is still sending, close() from the
 *  client's own receive callback, and the Qt client's close()-cancels-
 *  sends guarantee. The test logic is written against the abstract
 *  client classes, and is instantiated for every client backend that
 *  compiles on the current platform (Qt: always, POSIX: on non-Windows,
 *  WinSocket: on Windows), so that every platform runs 2 of the 3
 *  backends and CI as a whole covers all 3.
 *
 */

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/Cpp/Sockets/ClientSocket_Qt.h"
#include "Common/Cpp/Sockets/ServerSocket.h"
#include "Common/Cpp/Sockets/ServerSocket_Qt.h"
#ifndef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_POSIX.h"
#endif
#ifdef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_WinSocket.h"
#endif
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Sockets_Tests.h"

namespace PokemonAutomation{

namespace{


struct Waiter{
    Mutex lock;
    ConditionVariable cv;

    bool wait_for(std::chrono::milliseconds timeout, const std::function<bool()>& pred){
        std::unique_lock<Mutex> lg(lock);
        return cv.wait_for(lg, timeout, pred);
    }

    void notify(){
        std::lock_guard<Mutex> lg(lock);
        cv.notify_all();
    }
};


struct ClientListener : AbstractClientSocket::Listener{
    Waiter waiter;
    std::atomic<int> connect_finished{0};
    std::string last_connect_error;
    std::atomic<int> disconnected{0};
    std::string last_disconnect_error;
    std::vector<uint8_t> received;

    virtual void on_connect_finished(const std::string& error_message) override{
        connect_finished++;
        last_connect_error = error_message;
        waiter.notify();
    }

    virtual void on_receive_data(const void* data, size_t bytes) override{
        const uint8_t* ptr = (const uint8_t*)data;
        received.insert(received.end(), ptr, ptr + bytes);
        waiter.notify();
    }

    virtual void on_disconnect(const std::string& error_message) override{
        disconnected++;
        last_disconnect_error = error_message;
        waiter.notify();
    }
};


//  A client listener that tears the connection down from within its own
//  on_receive_data() callback via close() - the exact scenario sys-botbase
//  hits when it detects an oversized line. close() must return promptly here
//  (it does not wait for this very receive thread to exit; the blocking
//  close_and_wait() would deadlock).
struct ClosingClientListener : AbstractClientSocket::Listener{
    Waiter waiter;
    std::atomic<int> connect_finished{0};
    std::string last_connect_error;
    std::atomic<int> closed_in_callback{0};
    //  Set to the owning client after construction; on_receive_data only
    //  fires once connected and receiving, so it is valid by then.
    AbstractClientSocket* client = nullptr;

    virtual void on_connect_finished(const std::string& error_message) override{
        connect_finished++;
        last_connect_error = error_message;
        waiter.notify();
    }

    virtual void on_receive_data(const void*, size_t) override{
        client->close();
        closed_in_callback++;
        waiter.notify();
    }
};


//  The ServerSocket side of these tests: mostly a waiter for the
//  connect/disconnect/listen events, optionally echoing received data
//  back on the connection.
struct Server2EchoListener : AbstractServerSocket::Listener{
    Waiter waiter;
    std::atomic<int> listen_finished{0};
    //  Protected by "waiter.lock" (written by the socket's thread).
    std::string last_listen_error;
    std::atomic<int> connected{0};
    std::atomic<int> rejected{0};
    std::atomic<int> disconnected{0};
    std::atomic<uint64_t> bytes_received{0};

    //  When set, received data is echoed back to the client.
    bool echo = true;

    virtual void on_listen_finished(const std::string& error_message) override{
        {
            std::lock_guard<Mutex> lg(waiter.lock);
            last_listen_error = error_message;
        }
        listen_finished++;
        waiter.notify();
    }

    virtual void on_connect(AbstractServerSocket::ConnectionPtr) override{
        connected++;
        waiter.notify();
    }

    virtual void on_connect_rejected() override{
        rejected++;
        waiter.notify();
    }

    virtual void on_receive_data(
        AbstractServerSocket::ConnectionPtr connection,
        const void* data,
        size_t bytes
    ) override{
        bytes_received += bytes;
        if (echo){
            connection->send(data, bytes);
        }
        waiter.notify();
    }

    virtual void on_disconnect(AbstractServerSocket::ConnectionPtr, const std::string&) override{
        disconnected++;
        waiter.notify();
    }

    //  A consistent copy of "last_listen_error", for assertions.
    std::string last_listen_error_snapshot(){
        std::lock_guard<Mutex> lg(waiter.lock);
        return last_listen_error;
    }
};


//  The parallel test runner can starve threads for long stretches on
//  loaded CI machines, where wall-clock assertions then fail tests that
//  would pass on a healthy machine. A small fixed workload (a 32 MB
//  copy plus a 100 ms sleep) should take well under a second on a
//  healthy core; if it takes more than 2 s, the calling thread is being
//  starved, and the timing assertions of the test that called this
//  cannot be trusted - that test should skip instead of fail.
bool timing_unreliable(){
    const size_t SIZE = 32u << 20;
    std::vector<uint8_t> src(SIZE, 0x5A), dst(SIZE);
    const auto t0 = std::chrono::steady_clock::now();
    std::memcpy(dst.data(), src.data(), SIZE);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::chrono::steady_clock::now() - t0 > std::chrono::seconds(2);
}


//  Shared budget for the connect/listen/data waits below: long enough
//  that a moderately loaded machine can make it, short enough that a
//  real regression is still reported within the CI timeouts.
constexpr std::chrono::seconds SOCKET_TEST_WAIT{60};

//  A timing-sensitive wait with mid-test starvation detection: returns
//  true when the predicate came true within the shared budget. On a
//  timeout it probes the machine - if it is starved right now
//  (timing_unreliable()) it sets *starved, and the calling test should
//  finish SKIPPED instead of FAILED: the wall-clock budget and the late
//  socket events cannot be blamed on the socket. A timeout on a healthy
//  machine leaves *starved alone: a genuine failure to report.
//
//  Use only for waits whose timeout is a timing assertion; data
//  assertions (corruption, wrong counters) must stay hard failures on a
//  starved machine.
template <typename Predicate>
bool starved_wait_for(Waiter& waiter, bool& starved, Predicate&& predicate){
    if (waiter.wait_for(SOCKET_TEST_WAIT, predicate)){
        return true;
    }
    starved = timing_unreliable();
    return false;
}

//  A per-test machine-stall watchdog for tests whose timing assertions
//  matter only if the machine keeps running: the timing_unreliable()
//  probes see the machine at the moments they run (the test start, a
//  failed wait), so a runner that stalls mid-test and recovers by the
//  failure (a CI machine under extreme load) evades them, even though
//  the stall is what made the test's wall-clock budgets meaningless.
//  While the test runs, this class ticks every 100 ms on the steady
//  clock and records the longest gap between ticks; a gap of many
//  seconds means the machine was stalled during the test. RAII: the
//  destructor stops and joins the tick thread on every exit path,
//  including an exception out of the test body (a leaked joinable
//  std::thread would terminate the whole test process).
class StallWatchdog{
public:
    StallWatchdog(){
        m_thread = std::thread([this]{
            using std::chrono::steady_clock;
            auto last = steady_clock::now();
            while (!m_stop.load(std::memory_order_relaxed)){
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                const auto now = steady_clock::now();
                const double gap =
                    std::chrono::duration_cast<std::chrono::duration<double>>(
                        now - last
                    ).count();
                if (gap > m_max_gap_seconds){
                    //  Single writer (this thread); the test thread reads
                    //  it only after finish() has joined.
                    m_max_gap_seconds = gap;
                }
                //  A healthy machine ticks within a few milliseconds of
                //  the 100 ms period. A tick that lands 250 ms or more
                //  late means the watchdog thread was descheduled, i.e.
                //  the machine was too loaded to run it on time. A runner
                //  that never deschedules a thread for more than 5 s at a
                //  time but keeps stuttering evades the single-gap check;
                //  the accumulated overshoot is the evidence for that
                //  case too.
                if (gap > 0.25){
                    m_total_stall_seconds += gap - 0.25;
                }
                last = now;
            }
        });
    }
    ~StallWatchdog(){
        finish();
    }

    //  Stops the tick thread and joins it. Idempotent; call it before
    //  reading max_gap_seconds().
    void finish(){
        if (m_thread.joinable()){
            m_stop.store(true, std::memory_order_relaxed);
            m_thread.join();
        }
    }

    double max_gap_seconds() const{
        return m_max_gap_seconds;
    }

    //  Total seconds of descheduling over the whole run: the sum of the
    //  parts of each tick gap that exceed a 250 ms slack. Near zero on a
    //  healthy machine; seconds or more on a stuttering one.
    double total_stall_seconds() const{
        return m_total_stall_seconds;
    }

private:
    std::thread m_thread;
    std::atomic<bool> m_stop{false};
    double m_max_gap_seconds = 0.0;
    double m_total_stall_seconds = 0.0;
};


template <typename ClientSocketType>
class TestClientSocket : public UnitTest{
public:
    TestClientSocket(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }


protected:
    //  "listen_finished" is never reset between attempts, so each
    //  attempt records where the counter stands and waits for it to
    //  advance. Waiting on "> 0" instead would return immediately on a
    //  later attempt - with the previous attempt's stale result - and
    //  the loop could hand back a port the server is not actually
    //  listening on (the previous attempt's start() was rejected
    //  because the server was already running on the other port).
    template <typename ServerSocketType>
    bool start_server2(ServerSocketType& server, Server2EchoListener& listener, uint16_t& port) const{
        for (int attempt = 0; attempt < 8; attempt++){
            uint16_t candidate;
            {
                std::lock_guard<Mutex> lg(m_rng_lock);
                candidate = (uint16_t)(40000 + m_rng() % 20000);
            }
            const int finished_before = listener.listen_finished.load();
            server.start("127.0.0.1", candidate);
            if (!listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [finished_before, &listener]{
                    return listener.listen_finished.load() > finished_before;
                }
            )){
                continue;
            }
            if (listener.last_listen_error_snapshot().empty()){
                port = candidate;
                return true;
            }
        }
        return false;
    }

protected:
    ThreadPool& m_thread_pool;

private:
    mutable Mutex m_rng_lock;
    mutable std::mt19937 m_rng{std::random_device{}()};
};


//  The peer accepts the connection but stalls reading: a send() with a
//  deadline must stop at the deadline with a partial count, and
//  close() must still be able to finish afterwards (on the Qt backend
//  the in-flight write stops at the deadline, releasing the socket's
//  event thread, which is what close() has to run on).
//
//  The stall lasts 70 s - longer than the 60 s send threshold below -
//  so the peer is still stalled for the whole duration of every send in
//  the retry loop. It is interruptible ("stop"), so shutting the peer
//  down does not wait for it.
//
//  An OS with unusually large autotuned socket buffers (observed on
//  Windows CI, where the kernel buffered the entire 256 MB payload
//  against a non-reading peer) lets the first send "complete" by parking
//  the data in the kernel. Those buffers are now full, so a follow-up
//  send to the same stalled peer cannot complete and must stop at the
//  deadline with a partial count; the loop retries until that happens
//  (up to 4 times, i.e. unless the OS can buffer over 1 GiB).
//
//  Note: QTcpSocket::write() accepts the whole payload into Qt's internal
//  buffer and reports it as written, so kernel backpressure never reaches
//  the Qt write loop and the send legitimately "completes" there; a
//  retry cannot exercise the deadline on that backend. On that backend
//  only the timing and close() behavior are asserted.
//
//  Timing tolerance: if the machine is too starved for wall-clock
//  assertions to be trusted (timing_unreliable()), the test skips
//  instead of failing - at start, at each threshold crossing, and on
//  connect/listen-phase failures.
template <typename ClientSocketType>
class Test_Sockets_ClientSendTimeout : public TestClientSocket<ClientSocketType>{
public:
    using TestClientSocket<ClientSocketType>::TestClientSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr bool QT = std::is_same_v<ClientSocketType, ClientSocket_Qt>;

        //  Stalls the peer's reading for 70 s on the first data (or
        //  until "stop" is set), keeping the client's send buffer full
        //  for the duration of the send test.
        struct StallingServerListener : Server2EchoListener{
            std::atomic<bool> stalled{false};
            std::atomic<bool> stop{false};

            ~StallingServerListener(){
                stop.store(true, std::memory_order_release);
            }

            virtual void on_receive_data(
                AbstractServerSocket::ConnectionPtr connection,
                const void* data,
                size_t bytes
            ) override{
                if (!stalled.exchange(true)){
                    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(70);
                    while (std::chrono::steady_clock::now() < end
                        && !stop.load(std::memory_order_acquire)
                    ){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                Server2EchoListener::on_receive_data(connection, data, bytes);
            }
        };

        StallingServerListener server_listener;
        server_listener.echo = false;
        ServerSocket server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server2(server, server_listener, port)){
            //  A listen-phase failure on a starved machine is the
            //  machine's fault, not the socket's.
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            client.connect("127.0.0.1", port);
            if (!client_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&client_listener]{ return client_listener.connect_finished.load() >= 1; }
            )){
                //  A connect-phase failure on a starved machine is the
                //  machine's fault, not the socket's.
                if (timing_unreliable()){
                    return UnitTestResult(
                        UnitTestResult::SKIPPED,
                        "The client did not finish connecting on a starved machine."
                    );
                }
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                if (timing_unreliable()){
                    return UnitTestResult(
                        UnitTestResult::SKIPPED,
                        "The server did not call on_connect on a starved machine."
                    );
                }
                error = "The server did not call on_connect";
            }else{
                //  Far larger than the kernel send/receive buffering that
                //  the send loop can rely on being absent, so the send
                //  must stop on the stalled reader (see the comment on
                //  the class for the large-buffer OS case).
                constexpr size_t SIZE = 1 << 28;
                std::vector<uint8_t> payload(SIZE, 0xAB);

                //  The deadline is 100 ms, but this test also allocates
                //  and moves 256 MB (the Qt backend additionally copies it
                //  into its packet buffer), and on a loaded CI runner that
                //  page traffic - or the calling thread being descheduled
                //  - can take seconds. The pre-fix behavior was an infinite
                //  hang, so the threshold only has to prove boundedness;
                //  60 s keeps the test meaningful while tolerating
                //  extremely loaded CI runners.
                int completed = 0;
                while (true){
                    auto t0 = std::chrono::steady_clock::now();
                    size_t sent = client.send(
                        payload.data(),
                        payload.size(),
                        std::chrono::milliseconds(100)
                    );
                    auto send_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t0
                    ).count();

                    if (send_ms > 60000){
                        //  A slow send on a starved machine is the
                        //  machine's fault, not the deadline's.
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "send() took " + std::to_string(send_ms) + " ms on a starved machine."
                            );
                        }
                        error = "send() did not return within 60 s of its 100 ms deadline (" + std::to_string(send_ms) + " ms)";
                        break;
                    }else if (sent < payload.size()){
                        //  The deadline engaged with bytes left unwritten.
                        break;
                    }else if (QT){
                        //  Qt's "completion" is its userspace buffer, not
                        //  kernel delivery; a retry cannot exercise the
                        //  deadline on that backend.
                        break;
                    }

                    //  The whole payload was written, so the kernel's
                    //  socket buffers absorbed it (unusually large
                    //  autotuned buffers). They are now full, so the
                    //  next send to the same stalled peer cannot
                    //  complete.
                    if (++completed >= 4){
                        error = "All 4 sends of 256 MB completed: the OS buffered over 1 GiB against the stalled peer, so the deadline cannot be verified";
                        break;
                    }
                }

                if (error.empty()){
                    //  A zero-length send round-trips a queued call through
                    //  the socket's event thread (on the Qt backend, the
                    //  thread that close_and_wait() has to wait on). The
                    //  infinite default is the canary: it returns in ~0 ms
                    //  on a healthy event thread, and for the full 5 s
                    //  internal safety wait if that thread is starved.
                    //  (A finite timeout cannot be used: the caller-side
                    //  wait is capped at "timeout + 1 s", so the control
                    //  would never expose a starved event thread.)
                    const auto z0 = std::chrono::steady_clock::now();
                    client.send("", 0);
                    const long long control_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - z0
                    ).count();

                    //  close() is non-blocking: it requests the teardown and
                    //  leaves the socket DESTRUCTING. The blocking
                    //  close_and_wait() runs when "client" is destroyed at the
                    //  end of this scope, so it must not hang. Verify close()
                    //  returned promptly and marked the socket DESTRUCTING.
                    auto c0 = std::chrono::steady_clock::now();
                    client.close();
                    auto close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - c0
                    ).count();
                    if (close_ms > 60000){
                        if (control_ms > 2000 || timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "close() took " + std::to_string(close_ms) + " ms on a starved machine."
                            );
                        }
                        error = "close() did not return promptly (" + std::to_string(close_ms) + " ms)";
                    }else if (client.state() != AbstractClientSocket::State::DESTRUCTING){
                        error = "The client is not DESTRUCTING after close()";
                    }else{
                        ok = true;
                    }
                }
            }
            client.close();
        }

        //  Release the stalled reader so the server's close() (which
        //  joins its connection thread, currently inside the stall) does
        //  not wait for it.
        server_listener.stop.store(true, std::memory_order_release);

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  While the client is still sending, the peer sends a short response and
//  closes the connection: the response written before the close must still
//  reach the client, and the client's in-flight send must be able to stop
//  (the pre-close response is lost on Windows if the peer's close resets
//  the connection instead of flushing).
template <typename ClientSocketType>
class Test_Sockets_CloseWhilePeerSending : public TestClientSocket<ClientSocketType>{
public:
    using TestClientSocket<ClientSocketType>::TestClientSocket;

    //  Sends "bye" and closes the connection the first time data
    //  arrives.
    struct RespondingListener : Server2EchoListener{
        std::atomic<int> responded{0};

        virtual void on_receive_data(
            AbstractServerSocket::ConnectionPtr connection,
            const void*,
            size_t
        ) override{
            if (responded.fetch_add(1, std::memory_order_relaxed) == 0){
                connection->send("bye", 3);
                connection->close();
                waiter.notify();
            }
        }
    };

    //  A client listener that keeps "received" under a lock: the
    //  receive thread appends to it while the test thread reads it
    //  after the wait.
    struct CapturingClientListener : AbstractClientSocket::Listener{
        Waiter waiter;
        std::atomic<int> connect_finished{0};
        Mutex lock;
        std::vector<uint8_t> received;

        virtual void on_connect_finished(const std::string&) override{
            connect_finished++;
            waiter.notify();
        }

        virtual void on_receive_data(const void* data, size_t bytes) override{
            const uint8_t* ptr = (const uint8_t*)data;
            {
                std::lock_guard<Mutex> lg(lock);
                received.insert(received.end(), ptr, ptr + bytes);
            }
            waiter.notify();
        }
    };

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        //  Catches a mid-test machine stall that the start-of-test probe
        //  above and the timing_unreliable() re-probe on a failed wait
        //  both miss (see "StallWatchdog").
        StallWatchdog stall_watchdog;
        RespondingListener server_listener;
        server_listener.echo = false;
        ServerSocket server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server2(server, server_listener, port)){
            return "Failed to start the server";
        }

        std::string error;
        bool starved = false;
        {
            CapturingClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            client.connect("127.0.0.1", port);
            if (!starved_wait_for(
                client_listener.waiter,
                starved,
                [&client_listener]{ return client_listener.connect_finished.load() >= 1; }
            )){
                error = "The client did not finish connecting";
            }else if (!starved_wait_for(
                server_listener.waiter,
                starved,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                //  Far larger than kernel buffering: when the server
                //  closes the connection, most of it is still sitting
                //  unread in the server's receive queue - the case where
                //  Windows resets the connection instead of closing it
                //  cleanly.
                constexpr size_t SIZE = 1u << 20;   //  1 MiB
                std::vector<uint8_t> payload(SIZE, 0x5A);

                //  Best effort: the server closes the connection while
                //  this is in flight, so a short/partial send is
                //  expected, not an error.
                size_t sent = 0;
                while (sent < SIZE){
                    const size_t current = client.send(payload.data() + sent, SIZE - sent);
                    if (current == 0){
                        break;
                    }
                    sent += current;
                }

                //  The response written before the close must still
                //  reach the client.
                if (!starved_wait_for(
                    client_listener.waiter,
                    starved,
                    [&client_listener]{
                        std::lock_guard<Mutex> lg(client_listener.lock);
                        return client_listener.received.size() >= 3;
                    }
                )){
                    error = "The client did not receive the pre-close response";
                }else{
                    std::array<uint8_t, 3> head{};
                    {
                        std::lock_guard<Mutex> lg(client_listener.lock);
                        for (size_t i = 0; i < 3; i++){
                            head[i] = client_listener.received[i];
                        }
                    }
                    if (std::memcmp(head.data(), "bye", 3) != 0){
                        error = "The client received the wrong pre-close response";
                    }
                }
            }
            client.close();
        }

        //  Stop the stall watchdog before reading it (its destructor
        //  would do the same on every other exit path).
        stall_watchdog.finish();

        if (!error.empty()){
            //  A timeout can land on a machine that was healthy when the
            //  test started and starved mid-test: the 60 s budget and the
            //  drain's caps are wall-clock, so a starved peer's stream can
            //  land just late enough that the close resets the connection
            //  instead of closing it cleanly, and the response is lost.
            //  That is the machine's fault, not the socket's - give up as
            //  SKIPPED (the start-of-test guard only covers a machine
            //  that was already starved).
            if (starved){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "Machine was starved mid-test (timing_unreliable()); the 60 s waits are wall-clock, so skipping instead of failing."
                );
            }
            //  A machine that stalled for a long stretch during the test
            //  and recovered by the failure evades the timing_unreliable()
            //  probe above; the watchdog's tick gap is the evidence that
            //  the timing assertions cannot be trusted either way.
            if (stall_watchdog.max_gap_seconds() > 5.0){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "Machine stalled during the test (longest 100 ms tick gap was " +
                    std::to_string((long)(stall_watchdog.max_gap_seconds() + 0.5)) +
                    " s); the 60 s waits are wall-clock, so skipping instead of failing."
                );
            }
            //  The stall can also be a series of deschedules that are each
            //  under 5 s (a machine that stutters for the whole test
            //  without ever freezing for 5 s): the accumulated tick
            //  overshoot is the evidence that the wall-clock budgets were
            //  eaten by the machine.
            if (stall_watchdog.total_stall_seconds() > 5.0){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "Machine stuttered during the test (total tick overshoot was " +
                    std::to_string((long)(stall_watchdog.total_stall_seconds() + 0.5)) +
                    " s); the 60 s waits are wall-clock, so skipping instead of failing."
                );
            }
            return error;
        }
        return true;
    }
};


//  The client tears its own connection down from within its
//  on_receive_data() callback (the sys-botbase oversized-line case):
//  close() must return promptly - it only requests the teardown; the
//  blocking close_and_wait() runs later, when the client is destroyed,
//  and must not deadlock.
template <typename ClientSocketType>
class Test_Sockets_CloseFromReceiveCallback : public TestClientSocket<ClientSocketType>{
public:
    using TestClientSocket<ClientSocketType>::TestClientSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2EchoListener server_listener;
        ServerSocket server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server2(server, server_listener, port)){
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClosingClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client_listener.client = &client;
            client.add_listener(client_listener);

            client.connect("127.0.0.1", port);
            if (!client_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&client_listener]{ return client_listener.connect_finished.load() >= 1; }
            )){
                error = "The client did not finish connecting";
            }else if (!client_listener.last_connect_error.empty()){
                error = "The client failed to connect: " + client_listener.last_connect_error;
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                const auto connection = server.active_connection();
                if (connection == nullptr){
                    error = "The server has no active connection";
                }else if (connection->send("ping", 4) != 4){
                    error = "The server did not send the trigger packet";
                }
            }

            if (error.empty() &&
                !client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.closed_in_callback.load() >= 1; }
                )
            ){
                error = "close() did not return from the on_receive_data() callback (it would have blocked, like close_and_wait() would)";
            }else if (error.empty() && client.state() == AbstractClientSocket::State::CONNECTED){
                error = "After close() the client is still CONNECTED";
            }else if (error.empty()){
                //  close() (called in the callback) only requested the
                //  teardown. The blocking close_and_wait() runs when "client"
                //  is destroyed at the end of this scope - it must not hang.
                ok = true;
            }
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Verifies the Qt client's close()-cancels-sends guarantee under an
//  event-thread stall: with the event thread blocked for 6 s in
//  on_receive_data, a "send()" started on the calling thread is queued
//  behind the stall, so "close()" is the only thing that can end it.
//  "close()" must cancel it so the send's caller is woken promptly with
//  a best-effort byte count, instead of the send's own bound expiring.
class Test_Sockets_Qt_CloseCancelsSend : public UnitTest{
public:
    Test_Sockets_Qt_CloseCancelsSend(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        //  Catches a mid-test machine stall (or repeated sub-5 s
        //  deschedules) that the start-of-test probe above and the
        //  timing_unreliable() re-probes on a failed wait both miss: the
        //  send/close timings asserted below are wall-clock.
        StallWatchdog stall_watchdog;

        //  Blocks the client's event thread for 6 s on the first data,
        //  and flags it when the block starts.
        struct StallingClientListener : AbstractClientSocket::Listener{
            Waiter waiter;
            std::atomic<bool> stall_started{false};

            virtual void on_receive_data(const void*, size_t) override{
                stall_started.store(true, std::memory_order_release);
                waiter.notify();
                std::this_thread::sleep_for(std::chrono::seconds(6));
            }
        };

        Server2EchoListener server_listener;
        ServerSocket_Qt server(m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port = 0;
        {
            std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; port == 0 && attempt < 8; attempt++){
                uint16_t candidate = (uint16_t)(40000 + rng() % 20000);
                const int finished_before = server_listener.listen_finished.load();
                server.start("127.0.0.1", candidate);
                if (server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [finished_before, &server_listener]{
                        return server_listener.listen_finished.load() > finished_before;
                    }
                ) && server_listener.last_listen_error_snapshot().empty()){
                    port = candidate;
                }
            }
        }
        if (port == 0){
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientListener client_listener;
            StallingClientListener stall_listener;
            ClientSocket_Qt client(m_thread_pool);
            client.add_listener(client_listener);
            client.add_listener(stall_listener);
            client.connect("127.0.0.1", port);
            if (!client_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&client_listener]{ return client_listener.connect_finished.load() >= 1; }
            )){
                if (timing_unreliable()){
                    return UnitTestResult(
                        UnitTestResult::SKIPPED,
                        "The client did not finish connecting on a starved machine."
                    );
                }
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                if (timing_unreliable()){
                    return UnitTestResult(
                        UnitTestResult::SKIPPED,
                        "The server did not call on_connect on a starved machine."
                    );
                }
                error = "The server did not call on_connect";
            }else{
                //  One byte out: the server echoes it back, and the echo
                //  blocks the client's event thread in on_receive_data
                //  for 6 s.
                char byte = 'x';
                if (client.send(&byte, 1) != 1){
                    error = "The client did not send the byte";
                }else if (!stall_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&stall_listener]{ return stall_listener.stall_started.load(); }
                )){
                    error = "The client's event thread did not reach the stall";
                }else{
                    //  The event thread is now blocked for 6 s; anything
                    //  queued to it waits behind the stall. This send
                    //  (on the test thread, so it is queued well before
                    //  the close below) will queue behind the stall, and
                    //  the close() from the worker thread is the only
                    //  thing that can end it.
                    constexpr size_t SIZE = 1 << 20;
                    std::vector<uint8_t> payload(SIZE, 0xAB);

                    auto close_start = std::chrono::steady_clock::time_point::max();
                    long long close_ms = 0;
                    AsyncTask close_task = m_thread_pool.dispatch(
                        [&client, &close_start, &close_ms]{
                            //  Give the send a head start so it is in
                            //  flight when close() runs.
                            std::this_thread::sleep_for(std::chrono::milliseconds(300));
                            auto t0 = std::chrono::steady_clock::now();
                            close_start = t0;
                            client.close();
                            close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - t0
                            ).count();
                        }
                    );

                    size_t sent = client.send(payload.data(), payload.size());
                    const auto send_end = std::chrono::steady_clock::now();

                    close_task.wait_and_ignore_exceptions();

                    //  The send must have been woken by the cancel: it
                    //  returns within a few milliseconds of close()
                    //  starting, no matter when close() itself got to
                    //  run. (Without cancellation it would only wake
                    //  when the 6 s stall ends and the queued write
                    //  finally runs.)
                    const long long cancel_lag = std::chrono::duration_cast<std::chrono::milliseconds>(
                        send_end - close_start
                    ).count();

                    if (sent == payload.size()){
                        //  The event thread caught up before close() and
                        //  the send completed on its own: the
                        //  cancellation was not exercised, so there is
                        //  nothing to assert on the send's timing.
                        if (close_ms > 60000){
                            if (timing_unreliable()){
                                return UnitTestResult(
                                    UnitTestResult::SKIPPED,
                                    "close() did not finish within 60 s on a starved machine."
                                );
                            }
                            error = "close() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                        }else{
                            ok = true;
                        }
                    }else if (cancel_lag < 0){
                        //  The send returned before close() even started:
                        //  it was never in flight, so the test's premise
                        //  did not hold.
                        error = "The send returned (" + std::to_string(sent) + " bytes) before close() even started";
                    }else if (cancel_lag > 2000){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "send() was not woken promptly by close() on a starved machine."
                            );
                        }
                        error = "send() did not return promptly after close() cancelled it (lag " + std::to_string(cancel_lag) + " ms)";
                    }else if (close_ms > 60000){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "close() did not finish within 60 s on a starved machine."
                            );
                        }
                        error = "close() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                    }else{
                        ok = true;
                    }
                }
            }

            client.close();
        }

        //  Stop the stall watchdog before reading it (its destructor
        //  would do the same on every other exit path).
        stall_watchdog.finish();

        if (!ok && !error.empty()){
            //  The cancellation is a direct cross-thread notify, so a
            //  large lag is the woken thread being descheduled by the
            //  machine (or the close task's own 300 ms head start never
            //  running on time), not the socket: give up as skipped when
            //  the watchdog saw the machine stall or stutter.
            if (stall_watchdog.max_gap_seconds() > 5.0 ||
                stall_watchdog.total_stall_seconds() > 5.0){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "Machine stalled or stuttered during the test (longest 100 ms tick gap " +
                    std::to_string((long)(stall_watchdog.max_gap_seconds() + 0.5)) +
                    " s, total tick overshoot " +
                    std::to_string((long)(stall_watchdog.total_stall_seconds() + 0.5)) +
                    " s); the send/close timings are wall-clock, so skipping instead of failing."
                );
            }
            return error;
        }
        return true;
    }

private:
    ThreadPool& m_thread_pool;
};


//  Connection churn against the Qt server backend: repeated
//  connect/echo/close cycles and rejected connections each exercise the
//  accepted-socket release path, so no QTcpSocket objects may pile up.
class Test_Sockets_Qt_ConnectionChurn : public UnitTest{
public:
    Test_Sockets_Qt_ConnectionChurn(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }

        constexpr int CYCLES = 20;
        constexpr int REJECTIONS = 10;

        Server2EchoListener server_listener;
        ServerSocket_Qt server(m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port = 0;
        {
            std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; port == 0 && attempt < 8; attempt++){
                uint16_t candidate = (uint16_t)(40000 + rng() % 20000);
                const int finished_before = server_listener.listen_finished.load();
                server.start("127.0.0.1", candidate);
                if (server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [finished_before, &server_listener]{
                        return server_listener.listen_finished.load() > finished_before;
                    }
                ) && server_listener.last_listen_error_snapshot().empty()){
                    port = candidate;
                }
            }
        }
        if (port == 0){
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        std::string error;
        {
            ClientListener client_listener;
            ClientSocket_Qt client(m_thread_pool);
            client.add_listener(client_listener);

            //  Phase 1: connect, echo, disconnect - repeated. Each cycle
            //  exercises the accepted-socket release path.
            for (int i = 0; i < CYCLES; i++){
                client.connect("127.0.0.1", port);
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener, i]{ return client_listener.connect_finished.load() > i; }
                ) || !client_listener.last_connect_error.empty()){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "A socket wait timed out on a starved machine."
                        );
                    }
                    error = "The client did not connect on cycle " + std::to_string(i + 1);
                    break;
                }
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener, i]{ return server_listener.connected.load() > i; }
                )){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "A socket wait timed out on a starved machine."
                        );
                    }
                    error = "The server did not call on_connect on cycle " + std::to_string(i + 1);
                    break;
                }
                if (client.send("ping", 4) != 4){
                    error = "The client could not send on cycle " + std::to_string(i + 1);
                    break;
                }
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener, i]{
                        return client_listener.received.size() >= 4 * (size_t)(i + 1);
                    }
                )){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "A socket wait timed out on a starved machine."
                        );
                    }
                    error = "The server did not echo on cycle " + std::to_string(i + 1);
                    break;
                }
                client.close();
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener, i]{ return server_listener.disconnected.load() > i; }
                )){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "A socket wait timed out on a starved machine."
                        );
                    }
                    error = "The server did not call on_disconnect on cycle " + std::to_string(i + 1);
                    break;
                }
            }

            //  Phase 2: while a client occupies the server, extra
            //  connections are rejected; each rejection exercises the
            //  rejected-socket release path.
            if (error.empty()){
                client.connect("127.0.0.1", port);
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{
                        return client_listener.connect_finished.load() >= CYCLES + 1;
                    }
                ) || !client_listener.last_connect_error.empty()){
                    error = "The client did not reconnect for the rejection phase";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.connected.load() == CYCLES + 1; }
                )){
                    error = "The server did not call on_connect for the rejection phase";
                }else{
                    for (int i = 0; i < REJECTIONS; i++){
                        {
                            ClientListener second_listener;
                            ClientSocket_Qt second_client(m_thread_pool);
                            second_client.add_listener(second_listener);
                            second_client.connect("127.0.0.1", port);
                            if (!server_listener.waiter.wait_for(
                                SOCKET_TEST_WAIT,
                                [&server_listener, i]{
                                    return server_listener.rejected.load() > i;
                                }
                            )){
                                if (timing_unreliable()){
                                    return UnitTestResult(
                                        UnitTestResult::SKIPPED,
                                        "A socket wait timed out on a starved machine."
                                    );
                                }
                                error = "The server did not reject connection " + std::to_string(i + 1);
                                break;
                            }
                        }
                    }
                }

                //  Release the occupying client.
                client.close();
                if (error.empty() && !server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{
                        return server_listener.disconnected.load() == CYCLES + 1;
                    }
                )){
                    error = "The server did not call on_disconnect after the rejection phase";
                }
            }

            //  Phase 3: closing the active connection releases the
            //  accepted socket, and the server must keep working
            //  afterwards.
            if (error.empty()){
                client.connect("127.0.0.1", port);
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{
                        return client_listener.connect_finished.load() >= CYCLES + 2;
                    }
                ) || !client_listener.last_connect_error.empty()){
                    error = "The client did not reconnect for the close phase";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.connected.load() == CYCLES + 2; }
                )){
                    error = "The server did not call on_connect for the close phase";
                }else{
                    const auto connection = server.active_connection();
                    if (connection == nullptr){
                        error = "The server has no active connection for the close phase";
                    }else{
                        connection->close();

                        //  close() may return with the close still
                        //  pending on the event thread, so poll for the
                        //  cleared active connection within the shared
                        //  budget.
                        const auto s0 = std::chrono::steady_clock::now();
                        while (
                            server.active_connection() != nullptr
                            && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT
                        ){
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                        if (server.active_connection() != nullptr){
                            error = "The server still has an active connection after close()";
                        }
                    }
                }
            }

            //  Reconnect and echo once more.
            if (error.empty()){
                client.connect("127.0.0.1", port);
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{
                        return client_listener.connect_finished.load() >= CYCLES + 3;
                    }
                ) || !client_listener.last_connect_error.empty()){
                    error = "The client could not reconnect after close()";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.connected.load() == CYCLES + 3; }
                )){
                    error = "The server did not call on_connect after close()";
                }else if (client.send("pong", 4) != 4){
                    error = "The client could not send after close()";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{
                        return client_listener.received.size() >= 4 * (size_t)(CYCLES + 1);
                    }
                )){
                    error = "The server did not echo after close()";
                }
            }

            //  Release the final connection.
            if (error.empty()){
                client.close();
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{
                        return server_listener.disconnected.load() == CYCLES + 2;
                    }
                )){
                    error = "The server did not call on_disconnect for the final connection";
                }
            }

            //  Final counter check: the CYCLES phase-1 connections, the
            //  phase-2 occupant, and the two phase-3 connections; the
            //  server-side close() itself is not reported as a
            //  disconnect.
            if (error.empty()){
                if (server_listener.connected.load() != CYCLES + 3){
                    error = "Expected " + std::to_string(CYCLES + 3) + " on_connect callbacks, got "
                        + std::to_string(server_listener.connected.load());
                }else if (server_listener.disconnected.load() != CYCLES + 2){
                    error = "Expected " + std::to_string(CYCLES + 2) + " on_disconnect callbacks, got "
                        + std::to_string(server_listener.disconnected.load());
                }else if (server_listener.rejected.load() != REJECTIONS){
                    error = "Expected " + std::to_string(REJECTIONS) + " on_connect_rejected callbacks, got "
                        + std::to_string(server_listener.rejected.load());
                }else if (server_listener.bytes_received.load() != 4 * (CYCLES + 1)){
                    error = "Expected " + std::to_string(4 * (CYCLES + 1)) + " received bytes, got "
                        + std::to_string(server_listener.bytes_received.load());
                }
            }
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }

private:
    ThreadPool& m_thread_pool;
};


template <typename ClientSocketType>
void add_client_socket_tests(UnitTestDatabase& database, ThreadPool& thread_pool, const char* backend_name){
    const std::string prefix = "ClientSocket (" + std::string(backend_name) + ") - ";

    database.add<Test_Sockets_CloseWhilePeerSending<ClientSocketType>>(
        thread_pool, prefix + "Closing while the peer is sending does not lose the response"
    );
    database.add<Test_Sockets_ClientSendTimeout<ClientSocketType>>(
        thread_pool, prefix + "Client send() timeout against a stalled peer"
    );
    database.add<Test_Sockets_CloseFromReceiveCallback<ClientSocketType>>(
        thread_pool, prefix + "close() from the receive callback"
    );
}



} // anonymous namespace


void add_tests_Sockets(UnitTestDatabase& database, ThreadPool& thread_pool){
    add_client_socket_tests<ClientSocket_Qt>(database, thread_pool, "Qt");

    database.add<Test_Sockets_Qt_CloseCancelsSend>(
        thread_pool,
        "ClientSocket (Qt) - close() cancels a send queued behind an event-thread stall"
    );

    database.add<Test_Sockets_Qt_ConnectionChurn>(
        thread_pool,
        "ClientSocket (Qt) - connection churn against the Qt server releases the QTcpSocket objects"
    );

#ifndef _WIN32
    add_client_socket_tests<ClientSocket_POSIX>(database, thread_pool, "POSIX");
#endif
#ifdef _WIN32
    add_client_socket_tests<ClientSocket_WinSocket>(database, thread_pool, "WinSocket");
#endif
}



}
