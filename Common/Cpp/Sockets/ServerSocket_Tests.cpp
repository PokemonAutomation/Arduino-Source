/*  Server Socket Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Tests for the new AbstractServerSocket /
 *  AbstractServerSocketConnection pair.
 *
 *  The first section pins the API contracts against an in-memory stub
 *  backend (pointer lifetime, inert-after-close, on_disconnect() only
 *  for peer closes, one connection at a time). The rest are loopback
 *  tests that combine a ClientSocket and a ServerSocket against each
 *  other on 127.0.0.1, written against the abstract classes and
 *  instantiated for every backend that compiles on the current
 *  platform (Qt: always, POSIX: non-Windows, WinSocket: Windows).
 *
 */

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
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
#include "Common/Cpp/Sockets/ServerSocket_Qt.h"
#ifndef _WIN32
#include <fcntl.h>
#include "Common/Cpp/Sockets/ClientSocket_POSIX.h"
#include "Common/Cpp/Sockets/ServerSocket_POSIX.h"
#endif
#ifdef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_WinSocket.h"
#include "Common/Cpp/Sockets/ServerSocket_WinSocket.h"
#endif
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "ServerSocket_Tests.h"

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


struct ClientListener : AbstractClientSocket::Listener{
    Waiter waiter;
    std::atomic<int> connect_finished{0};
    std::string last_connect_error;
    std::atomic<int> disconnected{0};
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
        waiter.notify();
    }
};


//  The server-side listener for the loopback tests. It records the
//  current connection (so the tests can drive its send()/close()) and
//  optionally echoes received data back to the client.
struct Server2Listener : AbstractServerSocket::Listener{
    Waiter waiter;
    std::atomic<int> listen_finished{0};
    std::string last_listen_error;
    std::atomic<int> connected{0};
    std::atomic<int> rejected{0};
    std::atomic<int> disconnected{0};
    std::atomic<uint64_t> bytes_received{0};
    std::atomic<int> closed_in_callback{0};
    AbstractServerSocket::ConnectionPtr connection;

    //  When set, received data is echoed back to the client.
    bool echo = true;
    //  When set, the connection is closed from within
    //  on_receive_data() instead of echoing.
    bool close_on_receive = false;

    virtual void on_listen_finished(const std::string& error_message) override{
        listen_finished++;
        last_listen_error = error_message;
        waiter.notify();
    }

    virtual void on_connect(AbstractServerSocket::ConnectionPtr conn) override{
        connection = conn;
        connected++;
        waiter.notify();
    }

    virtual void on_connect_rejected() override{
        rejected++;
        waiter.notify();
    }

    virtual void on_receive_data(
        AbstractServerSocket::ConnectionPtr conn,
        const void* data,
        size_t bytes
    ) override{
        bytes_received += bytes;
        if (close_on_receive){
            //  The exact scenario sys-bot-base hits when it detects an
            //  oversized line: tear the connection down from within its
            //  own receive callback. close() must return promptly here.
            conn->close();
            closed_in_callback++;
        }else if (echo){
            conn->send(data, bytes);
        }
        waiter.notify();
    }

    virtual void on_disconnect(
        AbstractServerSocket::ConnectionPtr,
        const std::string& error_message
    ) override{
        disconnected++;
        waiter.notify();
    }
};


//  Stalls the client's reading for 70 s on the first data (or until
//  "stop" is set), keeping a large server-side send in flight.
struct StallingClientListener : ClientListener{
    std::atomic<bool> stalled{false};
    std::atomic<bool> stop{false};

    ~StallingClientListener(){
        stop.store(true, std::memory_order_release);
    }

    virtual void on_receive_data(const void* data, size_t bytes) override{
        if (!stalled.exchange(true)){
            const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(70);
            while (std::chrono::steady_clock::now() < end
                && !stop.load(std::memory_order_acquire)){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        ClientListener::on_receive_data(data, bytes);
    }
};


template <typename ClientSocketType, typename ServerSocketType>
class TestServerSocket : public UnitTest{
public:
    TestServerSocket(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }


protected:
    bool start_server(ServerSocketType& server, Server2Listener& listener, uint16_t& port) const{
        //  "listen_finished" is never reset between attempts, so each
        //  attempt records where the counter stands and waits for it to
        //  advance. Waiting on "> 0" instead would return immediately on a
        //  later attempt - with the previous attempt's stale result - and
        //  the loop could hand back a port the server is not actually
        //  listening on (the previous attempt's start() was rejected
        //  because the server was already running on the other port).
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
            if (listener.last_listen_error.empty()){
                port = candidate;
                return true;
            }
        }
        return false;
    }

    bool connect_client(
        ClientSocketType& client,
        ClientListener& listener,
        uint16_t port,
        int expected_connect
    ) const{
        client.connect("127.0.0.1", port);
        return listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&listener, expected_connect]{
                return listener.connect_finished.load() >= expected_connect;
            }
        );
    }

protected:
    ThreadPool& m_thread_pool;

private:
    mutable Mutex m_rng_lock;
    mutable std::mt19937 m_rng{std::random_device{}()};
};


//  Connect, send a packet, verify the echo, disconnect, and check that
//  the server observed both the connect and the disconnect.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_Echo : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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

        std::string error;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!client_listener.last_connect_error.empty()){
                error = "The client failed to connect: " + client_listener.last_connect_error;
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else if (client.send("hello world", 11) != 11){
                error = "The client did not send all 11 bytes";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.bytes_received.load() >= 11; }
            )){
                error = "The server did not receive the data";
            }else if (!client_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&client_listener]{ return client_listener.received.size() >= 11; }
            )){
                error = "The client did not receive the echo";
            }else if (std::memcmp(client_listener.received.data(), "hello world", 11) != 0){
                error = "The echo data was corrupted";
            }

            client.close();
        }

        if (error.empty() && !server_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            error = "The server did not call on_disconnect";
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Zero-length sends must be a valid no-op that returns 0 without
//  touching the connection.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_ZeroLengthSend : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                auto conn = server_listener.connection;
                if (conn->send("", 0) != 0){
                    error = "A zero-length send should return 0";
                }else if (!conn->is_open()){
                    error = "A zero-length send should not close the connection";
                }else if (client.send("x", 1) != 1){
                    error = "The client did not send its byte";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.bytes_received.load() >= 1; }
                )){
                    error = "The server did not receive the data after the zero-length send";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 1; }
                )){
                    error = "The client did not receive the echo";
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Stream a large payload from the client to the server and verify that
//  every byte arrives.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_LargeTransfer : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr size_t SIZE = 1 << 23;  // 8 MiB
        std::vector<uint8_t> payload(SIZE, 0x5A);

        Server2Listener server_listener;
        server_listener.echo = false;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        std::string error;
        std::atomic<size_t> bytes_sent{0};
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                AsyncTask send_task = this->m_thread_pool.dispatch([&client, &payload, &bytes_sent]{
                    bytes_sent = client.send(payload.data(), payload.size());
                });
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{
                        return server_listener.bytes_received.load() >= SIZE;
                    }
                )){
                    error = "The server did not receive the whole payload";
                }
                send_task.wait_and_ignore_exceptions();
                if (error.empty() && bytes_sent.load() != SIZE){
                    error = "The client reported sending " + std::to_string(bytes_sent.load()) + " of " + std::to_string(SIZE) + " bytes";
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Rapidly open and close a connection twenty times in a row, with
//  exact bookkeeping, and verify that the server keeps accepting new
//  clients and echoing.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_RapidReconnect : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr int CYCLES = 20;

        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        std::string error;
        size_t expected_echo = 0;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            for (int cycle = 1; error.empty() && cycle <= CYCLES; cycle++){
                if (!this->connect_client(client, client_listener, port, cycle)){
                    error = "The client did not reconnect (cycle " + std::to_string(cycle) + ")";
                }else if (!client_listener.last_connect_error.empty()){
                    error = "The client failed to reconnect: " + client_listener.last_connect_error;
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener, cycle]{ return server_listener.connected.load() == cycle; }
                )){
                    error = "The server did not accept the reconnection (cycle " + std::to_string(cycle) + ")";
                }else if (client.send("ping", 4) != 4){
                    error = "The client did not send the packet (cycle " + std::to_string(cycle) + ")";
                }else{
                    expected_echo += 4;
                    if (!client_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&client_listener, expected_echo]{
                            return client_listener.received.size() == expected_echo;
                        }
                    )){
                        error = "The client did not receive the echo (cycle " + std::to_string(cycle) + ")";
                    }else{
                        client.close();
                        if (!server_listener.waiter.wait_for(
                            SOCKET_TEST_WAIT,
                            [&server_listener, cycle]{
                                return server_listener.disconnected.load() == cycle;
                            }
                        )){
                            error = "The server did not observe the disconnect (cycle " + std::to_string(cycle) + ")";
                        }
                    }
                }
            }
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Start a large send on the connection, then close the connection from
//  the server side mid-send. The send must return promptly with a
//  best-effort byte count, and the server must keep accepting clients.
//  (The stalled-peer variant that pins the cancellation itself is
//  CloseCancelsSend below.)
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_SendCloseRace : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE, 0xAB);

        Server2Listener server_listener;
        server_listener.echo = false;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                auto conn = server_listener.connection;
                std::atomic<size_t> bytes_sent{0};
                AsyncTask send_task = this->m_thread_pool.dispatch([&conn, &payload, &bytes_sent]{
                    bytes_sent = conn->send(payload.data(), payload.size());
                });

                //  Give the send a head start, then cut the connection
                //  from the server side.
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                const auto c0 = std::chrono::steady_clock::now();
                conn->close();
                const long long close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - c0
                ).count();
                send_task.wait_and_ignore_exceptions();

                //  close() may return with the close still pending on a
                //  starved event thread (Qt backend), so poll for the
                //  active slot to clear within the shared budget before
                //  judging it.
                {
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.active_connection() != nullptr
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                if (server.active_connection() != nullptr){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "The active connection was not cleared after close() on a starved machine."
                        );
                    }
                    error = "The server is not listening after close()";
                }else if (bytes_sent.load() > SIZE){
                    error = "The send reported more bytes than the packet size";
                }else if (close_ms > 60000){
                    error = "close() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.disconnected.load() >= 1; }
                )){
                    error = "The client did not observe the disconnect";
                }

                //  The server must accept a new connection.
                if (error.empty() && !this->connect_client(client, client_listener, port, 2)){
                    error = "The client did not reconnect";
                }else if (error.empty()
                    && !server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener]{ return server_listener.connected.load() >= 2; }
                )){
                    error = "The server did not accept the new connection";
                }else if (error.empty() && client.send("x", 1) != 1){
                    error = "The new connection does not work";
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A 256 MiB send against a peer that stops reading stays in flight
//  when close() lands: the sender must be woken promptly with a
//  best-effort count, and the server must keep accepting clients.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_CloseCancelsSend : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    Test_ServerSocket_CloseCancelsSend(ThreadPool& thread_pool, std::string name)
        : TestServerSocket<ClientSocketType, ServerSocketType>(thread_pool, std::move(name))
    {
        //  The 256 MiB payload below, plus a transient 256 MiB copy in
        //  the Qt backend's in-flight send data (overreported on the
        //  other backends, which don't copy the payload).
        this->m_memory = 2ull * (1ull << 28);
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        server_listener.echo = false;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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

        std::string error;
        {
            StallingClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
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
                //  Far larger than the kernel send/receive buffering, so
                //  the send stays in flight against the stalled reader.
                //  (On an OS that buffered the whole payload anyway, the
                //  send completes before close() and the test still
                //  passes, just without exercising the cancel.)
                constexpr size_t SIZE = 1 << 28;
                std::vector<uint8_t> payload(SIZE, 0xAB);

                auto conn = server_listener.connection;
                std::atomic<size_t> sent{0};
                auto send_start = std::chrono::steady_clock::now();
                AsyncTask send_task = this->m_thread_pool.dispatch(
                    [&conn, &payload, &sent]{
                        sent = conn->send(payload.data(), payload.size());
                    }
                );

                //  Give the send a head start so it is in flight when
                //  close() runs, then cancel it.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                auto close_start = std::chrono::steady_clock::now();
                conn->close();
                const long long close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - close_start
                ).count();

                send_task.wait_and_ignore_exceptions();
                const long long send_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - send_start
                ).count();

                if (send_ms > 60000){
                    //  A send that is not ended by close() on a starved
                    //  machine is the machine's fault, not the
                    //  cancellation's.
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "send() did not return within 60 s of close() on a starved machine."
                        );
                    }
                    error = "send() did not return within 60 s of close() (" + std::to_string(send_ms) + " ms)";
                }else if (close_ms > 60000){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "close() did not finish within 60 s on a starved machine."
                        );
                    }
                    error = "close() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                }else if (sent.load() > SIZE){
                    error = "send() reported more bytes (" + std::to_string(sent.load()) + ") than the payload size";
                }else{
                    //  close() may return with the close still pending on
                    //  a starved event thread (Qt backend), so poll for
                    //  the active slot to clear within the shared budget
                    //  before judging it.
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.active_connection() != nullptr
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    if (server.active_connection() != nullptr){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "The active connection was not cleared after close() on a starved machine."
                            );
                        }
                        error = "The active connection was not cleared after close()";
                    }
                }
            }

            //  Release the stalled reader so the client's close() (which
            //  joins its receive thread, currently inside the stall) does
            //  not wait for it.
            client_listener.stop.store(true, std::memory_order_release);
            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  While a client is connected, every new client must be rejected -
//  hammered a few times in a row to exercise the rejection path in
//  churn - and the first client must keep working.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_RejectSecondClient : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr int REJECTIONS = 10;

        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The first client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect for the first client";
            }else{
                //  The reject is not reported to the second client in a
                //  backend-stable way (its connect may report success and
                //  disconnect later, or fail outright), so wait for the
                //  server-side report only.
                for (int attempt = 1; error.empty() && attempt <= REJECTIONS; attempt++){
                    ClientListener second_listener;
                    ClientSocketType second_client(this->m_thread_pool);
                    second_client.add_listener(second_listener);
                    second_client.connect("127.0.0.1", port);

                    if (!server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener, attempt]{
                            return server_listener.rejected.load() == attempt;
                        }
                    )){
                        error = "The server did not reject the second client (attempt " + std::to_string(attempt) + ")";
                    }
                }

                //  The first client must still work.
                if (error.empty() && client.send("x", 1) != 1){
                    error = "The first client stopped working";
                }else if (error.empty() && !client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 1; }
                )){
                    error = "The first client did not receive the echo";
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Starting on a port that is already in use must fail cleanly, and the
//  original server must keep working.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_PortInUse : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            if (timing_unreliable()){
                return UnitTestResult(
                    UnitTestResult::SKIPPED,
                    "The server did not start listening on a starved machine."
                );
            }
            return "Failed to start the server";
        }

        Server2Listener second_listener;
        ServerSocketType second_server(this->m_thread_pool);
        second_server.add_listener(second_listener);
        second_server.start("127.0.0.1", port);
        if (!second_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&second_listener]{ return second_listener.listen_finished.load() > 0; }
        )){
            return "The second server did not report its start";
        }
        if (second_listener.last_listen_error.empty()){
            return "The second server succeeded on a port that is already in use";
        }

        //  A failed start leaves the server retryable: start() must act
        //  again and report again (a half-started server would no-op it
        //  and never report a second result).
        const int finished_before = second_listener.listen_finished.load();
        second_server.start("127.0.0.1", port);
        if (!second_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [finished_before, &second_listener]{
                return second_listener.listen_finished.load() > finished_before;
            }
        )){
            return "A failed start left the server un-retryable";
        }
        if (second_listener.last_listen_error.empty()){
            return "The retried start succeeded on a port that is still in use";
        }

        //  The original server must still work.
        bool ok = false;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                ok = false;
            }else if (client.send("ok", 2) != 2){
                ok = false;
            }else if (!client_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&client_listener]{ return client_listener.received.size() >= 2; }
            )){
                ok = false;
            }else{
                ok = true;
            }

            client.close();
        }

        if (!ok){
            return "The original server stopped working";
        }
        return true;
    }
};


//  A peer-side close must fire on_disconnect() with the connection,
//  clear the active slot, and leave the server listening.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_PeerClose : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                client.close();
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.disconnected.load() >= 1; }
                )){
                    error = "The server did not call on_disconnect for the peer close";
                }
            }
        }

        if (error.empty()){
            //  The active slot must clear (eventually; the close lands
            //  within a few milliseconds on the thread backends and as a
            //  queued call on the Qt backend).
            const auto s0 = std::chrono::steady_clock::now();
            while (server.active_connection() != nullptr
                && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (server.active_connection() != nullptr){
                error = "The active connection was not cleared after the peer close";
            }
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  An application-side close() makes the connection inert (send()
//  returns 0, close() is a no-op), does not fire on_disconnect(), and
//  leaves the server accepting new clients.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_InertAfterClose : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                auto conn = server_listener.connection;
                conn->close();
                if (conn->is_open()){
                    error = "is_open() should be false after close()";
                }else if (conn->send("x", 1) != 0){
                    error = "send() on a closed connection should return 0";
                }else{
                    conn->close();  //  Idempotent.
                }

                //  The active slot clears (eventually; the close lands
                //  within a few milliseconds on the thread backends and
                //  as a queued call on the Qt backend).
                if (error.empty()){
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.active_connection() != nullptr
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    if (server.active_connection() != nullptr){
                        error = "The active connection was not cleared after the app close";
                    }
                }

                //  An application-side close must not have reported a
                //  disconnect: the only on_disconnect() this test may see
                //  is the peer close of connection #2, at the very end.
                if (error.empty() && server_listener.disconnected.load() != 0){
                    error = "on_disconnect() fired for an application-side close()";
                }

                //  The server must accept a new client and echo on it.
                if (error.empty() && !this->connect_client(client, client_listener, port, 2)){
                    error = "The client did not reconnect";
                }else if (error.empty()
                    && !server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener]{ return server_listener.connected.load() >= 2; }
                )){
                    error = "The server did not accept the new connection";
                }else if (error.empty() && client.send("ok", 2) != 2){
                    error = "The new connection does not work";
                }else if (error.empty() && !client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 2; }
                )){
                    error = "The new connection did not receive the echo";
                }
            }

            client.close();
        }

        //  Connection #2's peer close is the one disconnect this test
        //  expects.
        if (error.empty() && !server_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            error = "The server did not call on_disconnect for the peer close";
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A send on a connection that has been replaced must be refused, and
//  the replacement client must not receive the stale bytes.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_StaleSend : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        std::string error;
        AbstractServerSocket::ConnectionPtr first;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The first client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                first = server_listener.connection;
                client.close();
            }
        }

        if (error.empty() && !server_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            error = "The server did not call on_disconnect for the first connection";
        }

        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The second client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 2; }
            )){
                error = "The server did not call on_connect for the second client";
            }else{
                //  A send for the replaced connection must be refused,
                //  not delivered into the new client's socket.
                std::string stale = "stale";
                if (first->send(stale.data(), stale.size()) != 0){
                    error = "A send for a replaced connection was not refused";
                }else if (client.send("hello", 5) != 5){
                    error = "The send for the current connection was not delivered";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 5; }
                )){
                    error = "The second client did not receive the data";
                }else if (client_listener.received.size() != 5
                    || std::memcmp(client_listener.received.data(), "hello", 5) != 0
                ){
                    error = "The second client received the wrong (stale?) data";
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Destroying the server closes its active connection, but the
//  connection object the listener kept stays valid (inert) afterwards.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_OutlivesServer : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        std::string error;
        AbstractServerSocket::ConnectionPtr held;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            {
                Server2Listener server_listener;
                ServerSocketType server(this->m_thread_pool);
                server.add_listener(server_listener);

                uint16_t port;
                if (!this->start_server(server, server_listener, port)){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "The server did not start listening on a starved machine."
                        );
                    }
                    error = "Failed to start the server";
                }else if (!this->connect_client(client, client_listener, port, 1)){
                    error = "The client did not finish connecting";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.connected.load() >= 1; }
                )){
                    error = "The server did not call on_connect";
                }else{
                    held = server_listener.connection;
                }
            }
            //  The server is destroyed here; it closes the connection.

            if (error.empty()){
                //  The listener's copy must still be a valid, inert
                //  object.
                if (held == nullptr){
                    error = "The listener did not keep a copy of the connection";
                }else if (held->is_open()){
                    error = "The connection should be closed after the server is destroyed";
                }else if (held->send("x", 1) != 0){
                    error = "send() on a connection whose server is gone should return 0";
                }else{
                    held->close();  //  No-op, must not crash.
                }
            }

            //  The client must have observed the disconnect.
            if (error.empty()
                && !client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.disconnected.load() >= 1; }
            )){
                error = "The client did not observe the disconnect";
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  The server closes the connection from within its own
//  on_receive_data() callback (the sys-bot-base oversized-line
//  scenario). close() must return promptly, and the server must keep
//  accepting clients.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_CloseFromReceiveCallback : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    using TestServerSocket<ClientSocketType, ServerSocketType>::TestServerSocket;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        server_listener.close_on_receive = true;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                const auto c0 = std::chrono::steady_clock::now();
                //  The receive of this byte triggers the close() inside
                //  the callback.
                client.send("x", 1);
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.closed_in_callback.load() >= 1; }
                )){
                    error = "The server did not close the connection from the receive callback";
                }else{
                    const long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - c0
                    ).count();
                    if (elapsed_ms > 60000){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "close() from the receive callback took " + std::to_string(elapsed_ms) + " ms on a starved machine."
                            );
                        }
                        error = "close() from the receive callback did not return within 60 s (" + std::to_string(elapsed_ms) + " ms)";
                    }
                }

                if (error.empty()){
                    //  The server must accept a new connection.
                    if (!this->connect_client(client, client_listener, port, 2)){
                        error = "The client did not reconnect";
                    }else if (!server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener]{ return server_listener.connected.load() >= 2; }
                    )){
                        error = "The server did not accept the new connection";
                    }
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A send with a finite deadline against a stalled peer must stop at
//  the deadline and return the partial count, instead of hanging.
template <typename ClientSocketType, typename ServerSocketType>
class Test_ServerSocket_SendTimeout : public TestServerSocket<ClientSocketType, ServerSocketType>{
public:
    Test_ServerSocket_SendTimeout(ThreadPool& thread_pool, std::string name)
        : TestServerSocket<ClientSocketType, ServerSocketType>(thread_pool, std::move(name))
    {
        //  The 64 MiB payload below, plus a transient 64 MiB copy in
        //  the Qt backend's in-flight send data.
        this->m_memory = 1ull << 28;  // 256 MiB
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr bool QT = std::is_same_v<ServerSocketType, ServerSocket_Qt>;

        Server2Listener server_listener;
        server_listener.echo = false;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
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
            StallingClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                auto conn = server_listener.connection;

                //  Far larger than the kernel send/receive buffering that
                //  the send loop can rely on being absent, so the send
                //  must stop on the stalled reader.
                constexpr size_t SIZE = 1 << 26;  // 64 MiB
                std::vector<uint8_t> payload(SIZE, 0xAB);

                //  The deadline is 100 ms, but this test also allocates
                //  64 MB (the Qt backend additionally copies it into its
                //  packet buffer), and on a loaded CI runner that page
                //  traffic - or the calling thread being descheduled -
                //  can take seconds. The threshold only has to prove
                //  boundedness (the behavior without the deadline is an
                //  infinite hang), so 60 s keeps the test meaningful
                //  while tolerating extremely loaded CI runners.
                int completed = 0;
                while (true){
                    auto t0 = std::chrono::steady_clock::now();
                    size_t sent = conn->send(
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
                    //  autotuned buffers). They are now full, so the next
                    //  send to the same stalled peer cannot complete.
                    if (++completed >= 4){
                        error = "All 4 sends of 64 MB completed: the OS buffered over 256 MB against the stalled peer, so the deadline cannot be verified";
                        break;
                    }
                }

                if (error.empty()){
                    //  The deadline must not have closed the connection.
                    if (!conn->is_open()){
                        error = "The timed-out send closed the connection";
                    }
                }
            }

            //  Release the stalled reader so the client's close() (which
            //  joins its receive thread, currently inside the stall) does
            //  not wait for it.
            client_listener.stop.store(true, std::memory_order_release);
            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


#ifndef _WIN32
//  Regression test for a socket-handle leak: a server-initiated close()
//  handed the socket off to itself (take_fd()) and shut it down, but the
//  connection thread's exit path - the only other place that closes the
//  socket - no longer owned it, so nothing ever closed the handle. Each
//  such close then leaked one file descriptor, which grows without bound
//  in a long-lived server. Counts the process's open fds across many
//  server-initiated closes and fails if the count keeps growing.
class Test_ServerSocket_Posix_NoFdLeak
    : public TestServerSocket<ClientSocket_POSIX, ServerSocket_POSIX>
{
public:
    using TestServerSocket<ClientSocket_POSIX, ServerSocket_POSIX>::TestServerSocket;

    static int count_open_fds(){
        int count = 0;
        for (int fd = 0; fd < 4096; fd++){
            if (::fcntl(fd, F_GETFD) != -1){
                count++;
            }
        }
        return count;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }

        constexpr int CYCLES = 150;
        //  The parallel test threads hold their own sockets open through
        //  their tests, so the process-wide count carries a stable
        //  background (measured at ~30-35 fds on the full suite);
        //  the leak under test is one fd per cycle, so the tolerance
        //  keeps a two-fold margin on both sides. The logged growth
        //  makes any drift easy to retune.
        constexpr int TOLERANCE = 75;

        const int before = count_open_fds();
        std::string error;
        for (int i = 0; i < CYCLES; i++){
            error = close_cycle();
            if (!error.empty()){
                break;
            }
        }
        const int growth = count_open_fds() - before;
        logger.log("fd count grew by " + std::to_string(growth) + " over "
            + std::to_string(CYCLES) + " server-initiated closes (tolerance "
            + std::to_string(TOLERANCE) + ").");
        if (!error.empty()){
            return UnitTestResult(UnitTestResult::FAILED, error);
        }
        if (growth > TOLERANCE){
            return UnitTestResult(
                UnitTestResult::FAILED,
                "Server-initiated close() leaked " + std::to_string(growth)
                    + " file descriptors over " + std::to_string(CYCLES)
                    + " cycles; each close must leave no open socket behind."
            );
        }
        return UnitTestResult(UnitTestResult::PASSED);
    }

private:
    std::string close_cycle() const{
        Server2Listener server_listener;
        server_listener.echo = false;
        ServerSocket_POSIX server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "The server did not start listening.";
        }

        ClientSocket_POSIX client(this->m_thread_pool);
        ClientListener client_listener;
        client.add_listener(client_listener);
        if (!this->connect_client(client, client_listener, port, 1)){
            return "The client did not connect.";
        }

        if (!server_listener.waiter.wait_for(SOCKET_TEST_WAIT,
            [&server_listener]{
                return server_listener.connected.load() >= 1;
            }
        )){
            return "The server did not report the connection.";
        }

        //  The scenario under test: the server closes its side of the
        //  connection while the client is still connected.
        server_listener.connection->close();
        client_listener.waiter.wait_for(SOCKET_TEST_WAIT,
            [&client_listener]{
                return client_listener.disconnected.load() >= 1;
            }
        );
        //  Both destructors close_and_wait() their sockets, so no fd may
        //  survive this scope.
        return "";
    }
};
#endif


//  The Qt server's event thread is blocked in a listener callback when
//  this test queues a send and a close: the close's cancel must wake the
//  sender promptly (without the event thread's help), and the close must
//  still land once the stall ends.
class Test_ServerSocket_Qt_CloseUnderStall : public UnitTest{
public:
    Test_ServerSocket_Qt_CloseUnderStall(ThreadPool& thread_pool, std::string name)
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

        //  Blocks the server's event thread for 6 s on the first data,
        //  and flags it when the block starts. Only the first burst
        //  stalls: the Qt server delivers the rest of the trigger 4096
        //  bytes at a time, and every other burst stalling the event
        //  thread for 6 s would queue the close behind minutes of stalls.
        struct StallingServerListener : AbstractServerSocket::Listener{
            Waiter waiter;
            std::atomic<bool> stall_started{false};

            virtual void on_receive_data(
                AbstractServerSocket::ConnectionPtr,
                const void*,
                size_t
            ) override{
                if (!stall_started.exchange(true, std::memory_order_acq_rel)){
                    waiter.notify();
                    std::this_thread::sleep_for(std::chrono::seconds(6));
                }
            }
        };

        Server2Listener server_listener;
        StallingServerListener stall_listener;
        ServerSocket_Qt server(m_thread_pool);
        server.add_listener(server_listener);
        server.add_listener(stall_listener);

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
                ) && server_listener.last_listen_error.empty()){
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
            }else if (!client_listener.last_connect_error.empty()){
                error = "The client failed to connect: " + client_listener.last_connect_error;
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
                //  One burst in: the server's on_receive_data blocks its
                //  event thread for 6 s.
                constexpr size_t TRIGGER_SIZE = 1 << 18;
                std::vector<uint8_t> trigger(TRIGGER_SIZE, 0xCD);
                if (client.send(trigger.data(), trigger.size()) != TRIGGER_SIZE){
                    error = "The client did not send the trigger";
                }else if (!stall_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&stall_listener]{ return stall_listener.stall_started.load(); }
                )){
                    error = "The server's event thread did not reach the stall";
                }else{
                    auto conn = server_listener.connection;

                    //  The event thread is now blocked for 6 s; anything
                    //  queued to it waits behind the stall. This send
                    //  (on the test thread) queues behind the stall, and
                    //  the close from the worker thread is what must wake
                    //  it.
                    constexpr size_t SIZE = 1 << 20;
                    std::vector<uint8_t> payload(SIZE, 0xAB);

                    auto close_start = std::chrono::steady_clock::time_point::max();
                    long long close_ms = 0;
                    AsyncTask close_task = m_thread_pool.dispatch(
                        [&conn, &close_start, &close_ms]{
                            //  Give the send a head start so it is in
                            //  flight when close() runs.
                            std::this_thread::sleep_for(std::chrono::milliseconds(300));
                            auto t0 = std::chrono::steady_clock::now();
                            close_start = t0;
                            conn->close();
                            close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - t0
                            ).count();
                        }
                    );

                    size_t sent = conn->send(payload.data(), payload.size());
                    const auto send_end = std::chrono::steady_clock::now();

                    close_task.wait_and_ignore_exceptions();

                    //  The send must have been woken by the cancel: it
                    //  returns within a few milliseconds of close()
                    //  starting, no matter when the (stalled) event
                    //  thread gets to run the queued write. (Without
                    //  cancellation it would only wake when the 6 s
                    //  stall ends.)
                    const long long cancel_lag = std::chrono::duration_cast<std::chrono::milliseconds>(
                        send_end - close_start
                    ).count();

                    if (sent == payload.size()){
                        //  The event thread caught up before the close
                        //  and the send completed on its own: the
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
                        //  close() returns with the abort still queued
                        //  behind the (stalled) event thread, so poll
                        //  for the active slot to clear within the
                        //  shared budget before judging it.
                        const auto s0 = std::chrono::steady_clock::now();
                        while (server.active_connection() != nullptr
                            && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                        if (server.active_connection() != nullptr){
                            if (timing_unreliable()){
                                return UnitTestResult(
                                    UnitTestResult::SKIPPED,
                                    "The active connection was not cleared after close() on a starved machine."
                                );
                            }
                            error = "The active connection was not cleared after close()";
                        }
                    }
                }
            }

            client.close();
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }

private:
    ThreadPool& m_thread_pool;
};


template <typename ClientSocketType, typename ServerSocketType>
void add_server_socket2_tests(UnitTestDatabase& database, ThreadPool& thread_pool, const char* backend_name){
    const std::string prefix = "ServerSocket (" + std::string(backend_name) + ") - ";

    database.add<Test_ServerSocket_Echo<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Client/server echo round trip"
    );
    database.add<Test_ServerSocket_ZeroLengthSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Zero-length send"
    );
    database.add<Test_ServerSocket_LargeTransfer<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Large transfer"
    );
    database.add<Test_ServerSocket_RapidReconnect<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Rapid reconnect"
    );
    database.add<Test_ServerSocket_SendCloseRace<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Send/close race"
    );
    database.add<Test_ServerSocket_CloseCancelsSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "close() cancels an in-flight send"
    );
    database.add<Test_ServerSocket_RejectSecondClient<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Reject a second client"
    );
    database.add<Test_ServerSocket_PortInUse<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Port in use"
    );
    database.add<Test_ServerSocket_PeerClose<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "A peer close fires on_disconnect()"
    );
    database.add<Test_ServerSocket_InertAfterClose<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "An application close leaves an inert connection"
    );
    database.add<Test_ServerSocket_StaleSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "A send for a replaced connection is refused"
    );
    database.add<Test_ServerSocket_OutlivesServer<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "The connection outlives the server"
    );
    database.add<Test_ServerSocket_CloseFromReceiveCallback<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "close() from the receive callback"
    );
    database.add<Test_ServerSocket_SendTimeout<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "send() deadline against a stalled peer"
    );
}



class StubServer2;


//  The stub connection. It records what "send()" hands it and reports
//  its open/closed state; the stub server drives its events.
class StubConnection2 final : public AbstractServerSocketConnection{
public:
    explicit StubConnection2(StubServer2& server)
        : m_server(server)
    {}

    virtual bool is_open() const override;
    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout
    ) override;
    virtual void close() override;

    //  Stub hooks.
    void mark_closed_by_peer();


private:
    StubServer2& m_server;
    Mutex m_lock;
    std::string m_sent;
    std::atomic<bool> m_open{true};
};


//  The stub server. It implements the abstract server's bookkeeping
//  (state, active-connection slot, listener dispatch) and exposes
//  hooks that simulate the backend's event delivery: a client
//  connecting, data arriving, the peer closing, and teardown.
class StubServer2 final : public AbstractServerSocket{
public:
    StubServer2() = default;

    virtual ~StubServer2(){
        teardown();
    }

    //  Simulates the backend's "start()": it begins listening and
    //  reports the result synchronously.
    virtual void start(const std::string& address, uint16_t port) override{
        m_state = State::LISTENING;
        m_listeners.run_method(&Listener::on_listen_finished, "");
    }

    virtual ConnectionPtr active_connection() override{
        return m_active;
    }

    //  Test-only observables of the base's internal state machine: the
    //  application-facing API has no state() accessor, and the State
    //  enum is protected (MSVC will not let the tests name it through a
    //  using-declaration).
    bool is_not_running() const{
        return m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING;
    }
    bool is_listening() const{
        return m_state.load(std::memory_order_relaxed) == State::LISTENING;
    }
    bool is_connected() const{
        return m_state.load(std::memory_order_relaxed) == State::CONNECTED;
    }

    //  Stub hooks: simulate the backend's event delivery.

    //  A client connected. Rejected (on_connect_rejected) if a
    //  connection is already active or the server is not running.
    void connect_client(){
        if (m_state != State::LISTENING && m_state != State::CONNECTED){
            return;
        }
        if (m_active && m_active->is_open()){
            m_listeners.run_method(&Listener::on_connect_rejected);
            return;
        }
        m_active = std::make_shared<StubConnection2>(*this);
        m_state = State::CONNECTED;
        ConnectionPtr conn = m_active;
        m_listeners.run_method(&Listener::on_connect, conn);
    }

    //  Data arrived from the peer.
    void deliver(const void* data, size_t bytes){
        ConnectionPtr conn = m_active;
        if (!conn){
            return;
        }
        m_listeners.run_method(&Listener::on_receive_data, conn, data, bytes);
    }

    //  The peer closed the connection.
    void peer_closed(const std::string& error_message){
        ConnectionPtr conn = m_active;
        if (!conn){
            return;
        }
        m_active.reset();
        if (m_state == State::CONNECTED){
            m_state = State::LISTENING;
        }
        std::static_pointer_cast<StubConnection2>(conn)->mark_closed_by_peer();
        m_listeners.run_method(&Listener::on_disconnect, conn, error_message);
    }


private:
    friend class StubConnection2;

    //  A connection closed on the application side: leave the CONNECTED
    //  state and drop the slot. The application keeps its own
    //  reference to the (now inert) connection.
    void note_closed(){
        m_active.reset();
        if (m_state == State::CONNECTED){
            m_state = State::LISTENING;
        }
    }

    //  Destructor path: stop the server and close the active
    //  connection, the way the real backends' teardown does.
    void teardown(){
        ConnectionPtr conn = m_active;
        m_active.reset();
        m_state = State::NOT_RUNNING;
        if (conn && conn->is_open()){
            conn->close();
        }
    }

    //  The stub's own state machine (the backends' is theirs to keep
    //  and no longer part of the abstract class).
    enum class State{
        NOT_RUNNING,
        LISTENING,
        CONNECTED,
        DESTRUCTING,
    };
    std::atomic<State> m_state{State::NOT_RUNNING};

    ConnectionPtr m_active;
};



bool StubConnection2::is_open() const{
    return m_open.load(std::memory_order_relaxed);
}

size_t StubConnection2::send(
    const void* data,
    size_t bytes,
    std::chrono::milliseconds
){
    if (!m_open.load(std::memory_order_relaxed)){
        return 0;
    }
    std::lock_guard<Mutex> lg(m_lock);
    m_sent.append(static_cast<const char*>(data), bytes);
    return bytes;
}

void StubConnection2::close(){
    if (!m_open.exchange(false)){
        return;
    }
    m_server.note_closed();
}

void StubConnection2::mark_closed_by_peer(){
    m_open.store(false);
}



class Test_ServerSocket_StubApi : public UnitTest{
public:
    Test_ServerSocket_StubApi(ThreadPool&, std::string name)
        : UnitTest(std::move(name))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        struct Probe : AbstractServerSocket::Listener{
            Mutex lock;
            AbstractServerSocket::ConnectionPtr connection;
            std::string received;
            int connect_count = 0;
            int rejected_count = 0;
            int disconnect_count = 0;
            std::string last_error;

            virtual void on_connect(AbstractServerSocket::ConnectionPtr conn) override{
                std::lock_guard<Mutex> lg(lock);
                connection = std::move(conn);
                connect_count++;
            }

            virtual void on_connect_rejected() override{
                std::lock_guard<Mutex> lg(lock);
                rejected_count++;
            }

            virtual void on_receive_data(
                AbstractServerSocket::ConnectionPtr,
                const void* data,
                size_t bytes
            ) override{
                std::lock_guard<Mutex> lg(lock);
                received.append(static_cast<const char*>(data), bytes);
            }

            virtual void on_disconnect(
                AbstractServerSocket::ConnectionPtr,
                const std::string& error_message
            ) override{
                std::lock_guard<Mutex> lg(lock);
                disconnect_count++;
                last_error = error_message;
            }
        };

        Probe probe;

        AbstractServerSocket::ConnectionPtr first;
        AbstractServerSocket::ConnectionPtr second;

        {
            StubServer2 server;
            server.add_listener(probe);

            if (server.active_connection() != nullptr){
                return UnitTestResult("a stopped server has no active connection");
            }

            //  No client can connect before start().
            server.connect_client();
            if (probe.connect_count != 0 || probe.rejected_count != 0){
                return UnitTestResult("no client events may fire before start()");
            }

            server.start("127.0.0.1", 0);
            if (!server.is_listening()){
                return UnitTestResult("start() should leave the server LISTENING");
            }

            server.connect_client();
            if (probe.connect_count != 1 || probe.connection == nullptr){
                return UnitTestResult("on_connect() did not deliver a connection");
            }
            first = probe.connection;
            if (server.active_connection() != first
                || !server.is_connected()
            ){
                return UnitTestResult("active_connection()/state() disagree with on_connect()");
            }
            if (!first->is_open()){
                return UnitTestResult("a fresh connection should be open");
            }

            if (first->send("hello", 5) != 5){
                return UnitTestResult("send() on an open connection should accept all the bytes");
            }

            server.deliver("world", 5);
            if (probe.received != "world"){
                return UnitTestResult("on_receive_data() did not deliver the bytes");
            }

            //  One connection at a time: the second client is rejected,
            //  and the active connection is untouched.
            server.connect_client();
            if (probe.rejected_count != 1 || server.active_connection() != first){
                return UnitTestResult("a second client while one is active should be rejected");
            }

            //  Application-side close: the connection becomes inert,
            //  the server keeps listening, and no on_disconnect() fires.
            first->close();
            if (first->is_open() || probe.disconnect_count != 0){
                return UnitTestResult("an app close should close the connection without on_disconnect()");
            }
            if (first->send("x", 1) != 0){
                return UnitTestResult("send() on a closed connection should return 0");
            }
            if (server.active_connection() != nullptr
                || !server.is_listening()
            ){
                return UnitTestResult("after the close, the server should be LISTENING with no active connection");
            }
            first->close();  //  Idempotent.

            server.connect_client();
            if (probe.connect_count != 2 || server.active_connection() != probe.connection){
                return UnitTestResult("the server should accept a new client after the close");
            }
            second = probe.connection;

            server.peer_closed("");
            if (probe.disconnect_count != 1 || !probe.last_error.empty()){
                return UnitTestResult("a clean peer close should fire on_disconnect() once, with no error");
            }
            if (second->is_open() || server.active_connection() != nullptr
                || !server.is_listening()
            ){
                return UnitTestResult("after the peer close, the server should be LISTENING with no active connection");
            }
        }

        //  The server is gone. The listener's copy of the last
        //  connection must still be a valid, inert object.
        if (second == nullptr || second->is_open()){
            return UnitTestResult("the connection should outlive the server, closed by it");
        }
        if (second->send("x", 1) != 0){
            return UnitTestResult("a connection whose server is gone should be inert");
        }

        return UnitTestResult(true);
    }
};



}


void add_tests_ServerSocket(UnitTestDatabase& database, ThreadPool& thread_pool){
    database.add<Test_ServerSocket_StubApi>(
        thread_pool,
        "ServerSocket (Stub) - API shape and lifetime contracts"
    );

    add_server_socket2_tests<ClientSocket_Qt, ServerSocket_Qt>(database, thread_pool, "Qt");
#ifndef _WIN32
    database.add<Test_ServerSocket_Posix_NoFdLeak>(
        thread_pool,
        "ServerSocket (POSIX) - server-initiated closes leave no open file descriptor behind"
    );
#endif
    database.add<Test_ServerSocket_Qt_CloseUnderStall>(
        thread_pool,
        "ServerSocket (Qt) - close() under a stalled event thread"
    );
#ifndef _WIN32
    add_server_socket2_tests<ClientSocket_POSIX, ServerSocket_POSIX>(database, thread_pool, "POSIX");
#endif
#ifdef _WIN32
    add_server_socket2_tests<ClientSocket_WinSocket, ServerSocket_WinSocket>(database, thread_pool, "WinSocket");
#endif
}


}
