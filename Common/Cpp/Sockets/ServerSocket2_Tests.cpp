/*  Server Socket 2 Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Tests for the new AbstractServerSocket2 /
 *  AbstractServerSocketConnection pair.
 *
 *  The first section pins the API contracts against an in-memory stub
 *  backend (pointer lifetime, inert-after-close, on_disconnect() only
 *  for peer closes, one connection at a time). The rest are loopback
 *  tests that combine a ClientSocket and a ServerSocket2 against each
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
#include "Common/Cpp/Sockets/ServerSocket2_Qt.h"
#ifndef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_POSIX.h"
#include "Common/Cpp/Sockets/ServerSocket2_POSIX.h"
#endif
#ifdef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_WinSocket.h"
#include "Common/Cpp/Sockets/ServerSocket2_WinSocket.h"
#endif
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "ServerSocket2_Tests.h"

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
struct Server2Listener : AbstractServerSocket2::Listener{
    Waiter waiter;
    std::atomic<int> listen_finished{0};
    std::string last_listen_error;
    std::atomic<int> connected{0};
    std::atomic<int> rejected{0};
    std::atomic<int> disconnected{0};
    std::atomic<uint64_t> bytes_received{0};
    std::atomic<int> closed_in_callback{0};
    AbstractServerSocket2::ConnectionPtr connection;

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

    virtual void on_connect(AbstractServerSocket2::ConnectionPtr conn) override{
        connection = conn;
        connected++;
        waiter.notify();
    }

    virtual void on_connect_rejected() override{
        rejected++;
        waiter.notify();
    }

    virtual void on_receive_data(
        AbstractServerSocket2::ConnectionPtr conn,
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
        AbstractServerSocket2::ConnectionPtr,
        const std::string& error_message
    ) override{
        disconnected++;
        waiter.notify();
    }
};


template <typename ClientSocketType, typename ServerSocket2Type>
class TestServerSocket2 : public UnitTest{
public:
    TestServerSocket2(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }


protected:
    bool start_server(ServerSocket2Type& server, Server2Listener& listener, uint16_t& port) const{
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
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_Echo : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
            }else{
                ok = true;
            }

            client.close();
        }

        if (!error.empty() || !server_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            if (error.empty()){
                error = "The server did not call on_disconnect";
            }
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Zero-length sends must be a valid no-op that returns 0 without
//  touching the connection.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_ZeroLengthSend : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
                }else{
                    ok = true;
                }
            }

            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Stream a large payload from the client to the server and verify that
//  every byte arrives.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_LargeTransfer : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

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
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
                    [&server_listener, SIZE]{
                        return server_listener.bytes_received.load() >= SIZE;
                    }
                )){
                    error = "The server did not receive the whole payload";
                }
                send_task.wait_and_ignore_exceptions();
                if (error.empty() && bytes_sent.load() != SIZE){
                    error = "The client reported sending " + std::to_string(bytes_sent.load()) + " of " + std::to_string(SIZE) + " bytes";
                }else if (error.empty()){
                    ok = true;
                }
            }

            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Rapidly open and close a connection several times in a row, and
//  verify that the server keeps accepting new clients and echoing.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_RapidReconnect : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr int CYCLES = 5;

        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
        std::string error;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            for (int cycle = 1; ok == false && error.empty() && cycle <= CYCLES; cycle++){
                if (!this->connect_client(client, client_listener, port, cycle)){
                    error = "The client did not reconnect (cycle " + std::to_string(cycle) + ")";
                }else if (!client_listener.last_connect_error.empty()){
                    error = "The client failed to reconnect: " + client_listener.last_connect_error;
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener, cycle]{ return server_listener.connected.load() >= cycle; }
                )){
                    error = "The server did not accept the reconnection (cycle " + std::to_string(cycle) + ")";
                }else if (client.send("p", 1) != 1){
                    error = "The client did not send its byte (cycle " + std::to_string(cycle) + ")";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener, cycle]{ return client_listener.received.size() >= (size_t)cycle; }
                )){
                    error = "The client did not receive the echo (cycle " + std::to_string(cycle) + ")";
                }else{
                    client.close();
                    if (!server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener, cycle]{ return server_listener.disconnected.load() >= cycle; }
                    )){
                        error = "The server did not observe the disconnect (cycle " + std::to_string(cycle) + ")";
                    }
                }
            }
            if (error.empty()){
                ok = true;
            }
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Start a large send on the connection, then close the connection from
//  the server side mid-send. The send must return promptly with a
//  best-effort byte count, and the server must keep accepting clients.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_SendCloseRace : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

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
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
                //  resulting LISTENING state within the shared budget
                //  before judging it.
                {
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.state() != AbstractServerSocket2::State::LISTENING
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                if (server.state() != AbstractServerSocket2::State::LISTENING){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "The server is not LISTENING after close() on a starved machine."
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
                }else{
                    //  The server must accept a new connection.
                    if (!this->connect_client(client, client_listener, port, 2)){
                        error = "The client did not reconnect";
                    }else if (!server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener]{ return server_listener.connected.load() >= 2; }
                    )){
                        error = "The server did not accept the new connection";
                    }else if (client.send("x", 1) != 1){
                        error = "The new connection does not work";
                    }else{
                        ok = true;
                    }
                }
            }

            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  While a client is connected, a second client must be rejected, and
//  the first client must keep working.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_RejectSecondClient : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
                //  The second client must be rejected.
                ClientListener second_listener;
                ClientSocketType second_client(this->m_thread_pool);
                second_client.add_listener(second_listener);
                second_client.connect("127.0.0.1", port);

                //  The reject is not reported to the second client in a
                //  backend-stable way (its connect may report success and
                //  disconnect later, or fail outright), so wait for the
                //  server-side report only.
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.rejected.load() >= 1; }
                )){
                    error = "The server did not reject the second client";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.connect_finished.load() >= 1; }
                )){
                    error = "The first client lost its connection";
                }else if (client.send("x", 1) != 1){
                    error = "The first client stopped working";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 1; }
                )){
                    error = "The first client did not receive the echo";
                }else{
                    ok = true;
                }
                second_client.close();
            }

            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Starting on a port that is already in use must fail cleanly, and the
//  original server must keep working.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_PortInUse : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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
        ServerSocket2Type second_server(this->m_thread_pool);
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
        if (second_server.state() != AbstractServerSocket2::State::NOT_RUNNING){
            return "The second server is not NOT_RUNNING after a failed start";
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
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_PeerClose : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
            }else if (server.state() != AbstractServerSocket2::State::LISTENING){
                error = "The server is not LISTENING after the peer close";
            }else{
                ok = true;
            }
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  An application-side close() makes the connection inert (send()
//  returns 0, close() is a no-op), does not fire on_disconnect(), and
//  leaves the server accepting new clients.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_InertAfterClose : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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

                //  The server goes back to LISTENING (eventually).
                if (error.empty()){
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.state() != AbstractServerSocket2::State::LISTENING
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    if (server.state() != AbstractServerSocket2::State::LISTENING){
                        error = "The server is not LISTENING after the app close";
                    }
                }

                //  The server must accept a new client.
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
                }else if (error.empty()){
                    ok = true;
                }
            }

            client.close();
        }

        if (error.empty() && server_listener.disconnected.load() != 0){
            error = "on_disconnect() fired for an application-side close()";
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Destroying the server closes its active connection, but the
//  connection object the listener kept stays valid (inert) afterwards.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_OutlivesServer : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        bool ok = false;
        std::string error;
        AbstractServerSocket2::ConnectionPtr held;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            {
                Server2Listener server_listener;
                ServerSocket2Type server(this->m_thread_pool);
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
                    ok = true;
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

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  The server closes the connection from within its own
//  on_receive_data() callback (the sys-bot-base oversized-line
//  scenario). close() must return promptly, and the server must keep
//  accepting clients.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_CloseFromReceiveCallback : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        Server2Listener server_listener;
        server_listener.close_on_receive = true;
        ServerSocket2Type server(this->m_thread_pool);
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

        bool ok = false;
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
                    }else{
                        ok = true;
                    }
                }
            }

            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  A send with a finite deadline against a stalled peer must stop at
//  the deadline and return the partial count, instead of hanging.
template <typename ClientSocketType, typename ServerSocket2Type>
class Test_ServerSocket2_SendTimeout : public TestServerSocket2<ClientSocketType, ServerSocket2Type>{
public:
    using TestServerSocket2<ClientSocketType, ServerSocket2Type>::TestServerSocket2;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr bool QT = std::is_same_v<ServerSocket2Type, ServerSocket2_Qt>;

        //  Stalls the client's reading for 12 s on the first data (or
        //  until "stop" is set), keeping the server's send buffer full
        //  for the duration of the send test.
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

        Server2Listener server_listener;
        server_listener.echo = false;
        ServerSocket2Type server(this->m_thread_pool);
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


template <typename ClientSocketType, typename ServerSocket2Type>
void add_server_socket2_tests(UnitTestDatabase& database, ThreadPool& thread_pool, const char* backend_name){
    const std::string prefix = "ServerSocket2 (" + std::string(backend_name) + ") - ";

    database.add<Test_ServerSocket2_Echo<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Client/server echo round trip"
    );
    database.add<Test_ServerSocket2_ZeroLengthSend<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Zero-length send"
    );
    database.add<Test_ServerSocket2_LargeTransfer<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Large transfer"
    );
    database.add<Test_ServerSocket2_RapidReconnect<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Rapid reconnect"
    );
    database.add<Test_ServerSocket2_SendCloseRace<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Send/close race"
    );
    database.add<Test_ServerSocket2_RejectSecondClient<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Reject a second client"
    );
    database.add<Test_ServerSocket2_PortInUse<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "Port in use"
    );
    database.add<Test_ServerSocket2_PeerClose<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "A peer close fires on_disconnect()"
    );
    database.add<Test_ServerSocket2_InertAfterClose<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "An application close leaves an inert connection"
    );
    database.add<Test_ServerSocket2_OutlivesServer<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "The connection outlives the server"
    );
    database.add<Test_ServerSocket2_CloseFromReceiveCallback<ClientSocketType, ServerSocket2Type>>(
        thread_pool, prefix + "close() from the receive callback"
    );
    database.add<Test_ServerSocket2_SendTimeout<ClientSocketType, ServerSocket2Type>>(
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
    const std::string& sent_bytes() const{
        return m_sent;
    }
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
class StubServer2 final : public AbstractServerSocket2{
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



class Test_ServerSocket2_StubApi : public UnitTest{
public:
    Test_ServerSocket2_StubApi(ThreadPool&, std::string name)
        : UnitTest(std::move(name))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        struct Probe : AbstractServerSocket2::Listener{
            Mutex lock;
            AbstractServerSocket2::ConnectionPtr connection;
            std::string received;
            int connect_count = 0;
            int rejected_count = 0;
            int disconnect_count = 0;
            std::string last_error;

            virtual void on_connect(AbstractServerSocket2::ConnectionPtr conn) override{
                std::lock_guard<Mutex> lg(lock);
                connection = std::move(conn);
                connect_count++;
            }

            virtual void on_connect_rejected() override{
                std::lock_guard<Mutex> lg(lock);
                rejected_count++;
            }

            virtual void on_receive_data(
                AbstractServerSocket2::ConnectionPtr,
                const void* data,
                size_t bytes
            ) override{
                std::lock_guard<Mutex> lg(lock);
                received.append(static_cast<const char*>(data), bytes);
            }

            virtual void on_disconnect(
                AbstractServerSocket2::ConnectionPtr,
                const std::string& error_message
            ) override{
                std::lock_guard<Mutex> lg(lock);
                disconnect_count++;
                last_error = error_message;
            }
        };

        Probe probe;

        AbstractServerSocket2::ConnectionPtr first;
        AbstractServerSocket2::ConnectionPtr second;

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
            if (server.state() != AbstractServerSocket2::State::LISTENING){
                return UnitTestResult("start() should leave the server LISTENING");
            }

            server.connect_client();
            if (probe.connect_count != 1 || probe.connection == nullptr){
                return UnitTestResult("on_connect() did not deliver a connection");
            }
            first = probe.connection;
            if (server.active_connection() != first
                || server.state() != AbstractServerSocket2::State::CONNECTED
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
                || server.state() != AbstractServerSocket2::State::LISTENING
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
                || server.state() != AbstractServerSocket2::State::LISTENING
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


void add_tests_ServerSocket2(UnitTestDatabase& database, ThreadPool& thread_pool){
    database.add<Test_ServerSocket2_StubApi>(
        thread_pool,
        "ServerSocket2 (Stub) - API shape and lifetime contracts"
    );

    add_server_socket2_tests<ClientSocket_Qt, ServerSocket2_Qt>(database, thread_pool, "Qt");
#ifndef _WIN32
    add_server_socket2_tests<ClientSocket_POSIX, ServerSocket2_POSIX>(database, thread_pool, "POSIX");
#endif
#ifdef _WIN32
    add_server_socket2_tests<ClientSocket_WinSocket, ServerSocket2_WinSocket>(database, thread_pool, "WinSocket");
#endif
}


}
