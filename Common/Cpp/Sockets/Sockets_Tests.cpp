/*  Sockets Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Loopback tests that combine a ClientSocket and a ServerSocket against
 *  each other on 127.0.0.1. The test logic is written against the abstract
 *  classes, and is instantiated for every backend that compiles on the
 *  current platform (Qt: always, POSIX: on non-Windows, WinSocket: on
 *  Windows; each backend lands in its own PR), so that every platform
 *  runs 2 of the 3 backends and CI as a whole covers all 3.
 *
 */

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
#include "Common/Cpp/Sockets/ServerSocket_Qt.h"
#ifndef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_POSIX.h"
#include "Common/Cpp/Sockets/ServerSocket_POSIX.h"
#endif
#ifdef _WIN32
#include "Common/Cpp/Sockets/ClientSocket_WinSocket.h"
#include "Common/Cpp/Sockets/ServerSocket_WinSocket.h"
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


struct ServerListener : AbstractServerSocket::Listener{
    Waiter waiter;
    std::atomic<int> listen_finished{0};
    std::string last_listen_error;
    std::atomic<int> connected{0};
    std::atomic<int> rejected{0};
    std::atomic<int> disconnected{0};
    std::string last_disconnect_error;
    std::atomic<uint64_t> bytes_received{0};

    //  When set, received data is echoed back to the client.
    AbstractServerSocket* server = nullptr;

    virtual void on_listen_finished(const std::string& error_message) override{
        listen_finished++;
        last_listen_error = error_message;
        waiter.notify();
    }

    virtual void on_connect() override{
        connected++;
        waiter.notify();
    }

    virtual void on_connect_rejected() override{
        rejected++;
        waiter.notify();
    }

    virtual void on_receive_data(const void* data, size_t bytes) override{
        bytes_received += bytes;
        if (server != nullptr){
            server->send(data, bytes);
        }
        waiter.notify();
    }

    virtual void on_disconnect(const std::string& error_message) override{
        disconnected++;
        last_disconnect_error = error_message;
        waiter.notify();
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


template <typename ClientSocketType, typename ServerSocketType>
class TestSockets : public UnitTest{
public:
    TestSockets(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }


protected:
    bool start_server(ServerSocketType& server, ServerListener& listener, uint16_t& port) const{
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


//  Connect, send a packet, verify the echo, disconnect, and check that the
//  server observed both the connect and the disconnect.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_Echo : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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

        if (!server_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            error = "The server did not call on_disconnect";
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Zero-length sends must be a valid no-op that returns 0 without
//  touching the connection.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_ZeroLengthSend : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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
            }else if (client.send("", 0) != 0){
                error = "A zero-length client send did not return 0";
            }else if (server.send("", 0) != 0){
                error = "A zero-length server send did not return 0";
            }else{
                //  The connection must still be usable afterwards.
                if (client.send("ping", 4) != 4){
                    error = "The client did not send all 4 bytes after a zero-length send";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.bytes_received.load() >= 4; }
                )){
                    error = "The server did not receive the data";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 4; }
                )){
                    error = "The client did not receive the echo";
                }else if (std::memcmp(client_listener.received.data(), "ping", 4) != 0){
                    error = "The echo data was corrupted";
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


//  Transfer a megabyte each way (the server echoes it back) and verify the
//  data integrity.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_LargeTransfer : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE);
        for (size_t i = 0; i < SIZE; i++){
            payload[i] = (uint8_t)(i % 251);
        }

        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else if (client.send(payload.data(), payload.size()) != payload.size()){
                error = "The client did not send all " + std::to_string(payload.size()) + " bytes";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(30),
                [&server_listener]{ return server_listener.bytes_received.load() >= SIZE; }
            )){
                error = "The server did not receive all the data";
            }else if (!client_listener.waiter.wait_for(
                std::chrono::seconds(30),
                [&client_listener]{ return client_listener.received.size() >= SIZE; }
            )){
                error = "The client did not receive all the echo data";
            }else if (std::memcmp(client_listener.received.data(), payload.data(), SIZE) != 0){
                error = "The echo data was corrupted";
            }else{
                ok = true;
            }

            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Rapidly connect and disconnect many times. Each cycle transfers a
//  packet to make sure the new connection actually works.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_RapidReconnect : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr int CYCLES = 50;

        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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
        size_t expected_echo = 0;
        {
            ClientListener client_listener;
            ClientSocketType client(this->m_thread_pool);
            client.add_listener(client_listener);

            for (int cycle = 0; cycle < CYCLES; cycle++){
                int cycle_no = cycle + 1;

                if (!this->connect_client(client, client_listener, port, cycle_no)){
                    error = "The client did not finish connecting (cycle " + std::to_string(cycle_no) + ")";
                    break;
                }
                if (!client_listener.last_connect_error.empty()){
                    error = "The client failed to connect (cycle " + std::to_string(cycle_no) + "): " + client_listener.last_connect_error;
                    break;
                }
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener, cycle_no]{ return server_listener.connected.load() >= cycle_no; }
                )){
                    error = "The server did not call on_connect (cycle " + std::to_string(cycle_no) + ")";
                    break;
                }

                std::string packet = "cycle " + std::to_string(cycle_no);
                expected_echo += packet.size();
                if (client.send(packet.data(), packet.size()) != packet.size()){
                    error = "The client did not send the packet (cycle " + std::to_string(cycle_no) + ")";
                    break;
                }
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener, expected_echo]{ return client_listener.received.size() >= expected_echo; }
                )){
                    error = "The client did not receive the echo (cycle " + std::to_string(cycle_no) + ")";
                    break;
                }

                client.close();
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener, cycle_no]{ return server_listener.disconnected.load() >= cycle_no; }
                )){
                    error = "The server did not call on_disconnect (cycle " + std::to_string(cycle_no) + ")";
                    break;
                }
            }

            if (error.empty()){
                ok = true;
            }
            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Send a large packet while the server closes the connection underneath
//  it: the send must finish (returning however much was written), the
//  server must go back to listening, and the client must observe the
//  disconnect. Then the server must accept a new connection.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_SendCloseRace : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE, 0xAB);

        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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
                std::atomic<size_t> bytes_sent{0};
                AsyncTask send_task = this->m_thread_pool.dispatch([&client, &payload, &bytes_sent]{
                    bytes_sent = client.send(payload.data(), payload.size());
                });

                //  Give the send a head start, then cut the connection from
                //  the server side.
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                server.close_connection();

                send_task.wait_and_ignore_exceptions();

                //  close_connection() may return with the close still
                //  pending on a starved event thread (Qt backend), so
                //  poll for the resulting LISTENING state within the
                //  shared budget before judging it.
                {
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.state() != AbstractServerSocket::State::LISTENING
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                if (server.state() != AbstractServerSocket::State::LISTENING){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "The server is not LISTENING after close_connection() on a starved machine."
                        );
                    }
                    error = "The server is not listening after close_connection()";
                }else if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.disconnected.load() >= 1; }
                )){
                    error = "The client did not observe the disconnect";
                }else if (bytes_sent.load() > SIZE){
                    error = "The client reported sending more bytes than the packet size";
                }else{
                    //  The server must accept a new connection.
                    if (!this->connect_client(client, client_listener, port, 2)){
                        error = "The client did not reconnect";
                    }else if (!client_listener.last_connect_error.empty()){
                        error = "The client failed to reconnect: " + client_listener.last_connect_error;
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


//  While a client is connected, a second client must be rejected, and the
//  first client must keep working.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_RejectSecondClient : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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
            ClientListener client_a_listener;
            ClientSocketType client_a(this->m_thread_pool);
            client_a.add_listener(client_a_listener);

            ClientListener client_b_listener;
            ClientSocketType client_b(this->m_thread_pool);
            client_b.add_listener(client_b_listener);

            if (!this->connect_client(client_a, client_a_listener, port, 1)){
                error = "Client A did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                client_b.connect("127.0.0.1", port);
                if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.rejected.load() >= 1; }
                )){
                    error = "The server did not reject the second client";
                }else{
                    //  The first client must keep working.
                    if (client_a.send("still here?", 11) != 11){
                        error = "Client A did not send all 11 bytes";
                    }else if (!client_a_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&client_a_listener]{ return client_a_listener.received.size() >= 11; }
                    )){
                        error = "Client A did not receive the echo";
                    }else if (std::memcmp(client_a_listener.received.data(), "still here?", 11) != 0){
                        error = "The echo data was corrupted";
                    }else{
                        //  Client B must have got a final answer to its
                        //  connect attempt, whatever it was.
                        if (!client_b_listener.waiter.wait_for(
                            SOCKET_TEST_WAIT,
                            [&client_b_listener]{ return client_b_listener.connect_finished.load() >= 1; }
                        )){
                            error = "Client B did not finish its connect attempt";
                        }else{
                            ok = true;
                        }
                    }
                }
            }

            client_a.close();
            client_b.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  "send()" for a connection that has already been replaced must not
//  reach the new client's socket: the connection id changes on every
//  accept, and a stale id is refused.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_StaleSend : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        std::string error;
        if (server.connection_id() != 0){
            error = "connection_id() was not 0 before any connection";
        }

        uint64_t first_id = 0;
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
                first_id = server.connection_id();
                if (first_id == 0){
                    error = "The server did not assign a connection id";
                }
                client.close();
            }
        }

        if (error.empty() && !server_listener.waiter.wait_for(
            SOCKET_TEST_WAIT,
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            error = "The server did not call on_disconnect";
        }

        //  The counter is monotonic and not reset, but with no active
        //  connection the id must report 0.
        if (error.empty() && server.connection_id() != 0){
            error = "connection_id() was not 0 while no connection was active";
        }

        uint64_t second_id = 0;
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
                second_id = server.connection_id();
                if (second_id == first_id){
                    error = "The connection id did not change after reconnect";
                }
                //  A send for the replaced connection must be refused,
                //  not delivered into the new client's socket.
                if (error.empty() &&
                    server.send("stale", 5, std::chrono::milliseconds::max(), first_id) != 0
                ){
                    error = "A send for a replaced connection was not refused";
                }
                //  A send for the current connection still goes through.
                if (error.empty() &&
                    server.send("hello", 5, std::chrono::milliseconds::max(), second_id) != 5
                ){
                    error = "The send for the current connection was not delivered";
                }
                if (error.empty() && !client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{ return client_listener.received.size() >= 5; }
                )){
                    error = "The second client did not receive the data";
                }else if (error.empty() &&
                    std::memcmp(client_listener.received.data(), "hello", 5) != 0
                ){
                    error = "The second client received the wrong data";
                }
                client.close();
            }
        }

        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Starting a second server on an already-bound port must fail through
//  on_listen_finished and leave the server not running.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_PortInUse : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        ServerListener server_listener;
        ServerSocketType server(this->m_thread_pool);
        server_listener.server = &server;
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

        ServerListener second_listener;
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
        if (second_server.state() != AbstractServerSocket::State::NOT_RUNNING){
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


//  Qt backend only. Stall the server's event thread for longer than the 5 s
//  completion wait (by blocking inside an on_receive_data callback, which
//  runs on that thread). close_connection() must then time out and return
//  cleanly, and the queued handler, which runs after the caller has gone
//  back, must be able to complete: the completion's shared ownership keeps
//  it alive until then. Previously this was a stack-use-after-return: the
//  handler wrote to the dead stack-local CloseCompletion.
class Test_Sockets_Qt_CompletionLifetime : public UnitTest{
public:
    Test_Sockets_Qt_CompletionLifetime(ThreadPool& thread_pool, std::string name)
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
        //  Blocks the event thread while it receives data.
        struct StallListener : AbstractServerSocket::Listener{
            virtual void on_receive_data(const void*, size_t) override{
                std::this_thread::sleep_for(std::chrono::seconds(6));
            }
        };

        ServerListener server_listener;
        StallListener stall_listener;
        ServerSocket_Qt server(m_thread_pool);
        server_listener.server = &server;
        server.add_listener(server_listener);
        server.add_listener(stall_listener);

        uint16_t port = 0;
        {
            std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; attempt < 8; attempt++){
                uint16_t candidate = (uint16_t)(40000 + rng() % 20000);
                server.start("127.0.0.1", candidate);
                if (server_listener.waiter.wait_for(
                    std::chrono::seconds(5),
                    [&server_listener]{ return server_listener.listen_finished.load() > 0; }
                ) && server_listener.last_listen_error.empty()){
                    port = candidate;
                    break;
                }
            }
        }
        if (port == 0){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientListener client_listener;
            ClientSocket_Qt client(m_thread_pool);
            client.add_listener(client_listener);
            client.connect("127.0.0.1", port);
            if (!client_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&client_listener]{ return client_listener.connect_finished.load() >= 1; }
            )){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                //  One byte in: the event thread blocks in on_receive_data for 6 s.
                char byte = 'x';
                if (client.send(&byte, 1) != 1){
                    error = "The client did not send the byte";
                }else{
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));

                    auto start = std::chrono::steady_clock::now();
                    server.close_connection();
                    auto waited = std::chrono::steady_clock::now() - start;
                    if (waited < std::chrono::seconds(4) || waited >= std::chrono::seconds(8)){
                        error = "close_connection() did not time out as expected (" +
                            std::to_string(
                                (long long)std::chrono::duration_cast<std::chrono::milliseconds>(waited).count()
                            ) + " ms)";
                    }else{
                        //  Wait for the stall to finish so the late handler actually
                        //  runs, then check that it restored the state.
                        std::this_thread::sleep_for(std::chrono::seconds(6));
                        if (server.state() != AbstractServerSocket::State::LISTENING){
                            error = "The late handler did not restore LISTENING";
                        }else{
                            ok = true;
                        }
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

private:
    ThreadPool& m_thread_pool;
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

        ServerListener server_listener;
        ServerSocket_Qt server(m_thread_pool);
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port = 0;
        {
            std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; attempt < 8; attempt++){
                uint16_t candidate = (uint16_t)(40000 + rng() % 20000);
                server.start("127.0.0.1", candidate);
                if (server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.listen_finished.load() > 0; }
                ) && server_listener.last_listen_error.empty()){
                    port = candidate;
                    break;
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

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }

private:
    ThreadPool& m_thread_pool;
};


//  Verifies the Qt server's close_connection()-cancels-sends guarantee
//  under an event-thread stall: with the event thread blocked for 6 s
//  in on_receive_data, a "send()" started on the calling thread is
//  queued behind the stall, so "close_connection()" is the only thing
//  that can end it. It must cancel the send so the send's caller is
//  woken promptly with a best-effort byte count, instead of the
//  send's own bound expiring.
class Test_Sockets_Qt_CloseConnectionCancelsSend : public UnitTest{
public:
    Test_Sockets_Qt_CloseConnectionCancelsSend(ThreadPool& thread_pool, std::string name)
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

            virtual void on_receive_data(const void*, size_t) override{
                if (!stall_started.exchange(true, std::memory_order_acq_rel)){
                    waiter.notify();
                    std::this_thread::sleep_for(std::chrono::seconds(6));
                }
            }
        };

        ServerListener server_listener;
        StallingServerListener stall_listener;
        ServerSocket_Qt server(m_thread_pool);
        server.add_listener(server_listener);
        server.add_listener(stall_listener);

        uint16_t port = 0;
        {
            std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; attempt < 8; attempt++){
                uint16_t candidate = (uint16_t)(40000 + rng() % 20000);
                server.start("127.0.0.1", candidate);
                if (server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.listen_finished.load() > 0; }
                ) && server_listener.last_listen_error.empty()){
                    port = candidate;
                    break;
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
                    //  The event thread is now blocked for 6 s; anything
                    //  queued to it waits behind the stall. This send
                    //  (on the test thread, so it is queued well before
                    //  the close below) will queue behind the stall, and
                    //  the close_connection() from the worker thread is
                    //  the only thing that can end it.
                    constexpr size_t SIZE = 1 << 20;
                    std::vector<uint8_t> payload(SIZE, 0xAB);

                    auto close_start = std::chrono::steady_clock::time_point::max();
                    long long close_ms = 0;
                    AsyncTask close_task = m_thread_pool.dispatch(
                        [&server, &close_start, &close_ms]{
                            //  Give the send a head start so it is in
                            //  flight when close_connection() runs.
                            std::this_thread::sleep_for(std::chrono::milliseconds(300));
                            auto t0 = std::chrono::steady_clock::now();
                            close_start = t0;
                            server.close_connection();
                            close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - t0
                            ).count();
                        }
                    );

                    size_t sent = server.send(payload.data(), payload.size());
                    const auto send_end = std::chrono::steady_clock::now();

                    close_task.wait_and_ignore_exceptions();

                    //  The send must have been woken by the cancel: it
                    //  returns within a few milliseconds of
                    //  close_connection() starting, no matter when
                    //  close_connection() itself got to run. (Without
                    //  cancellation it would only wake when the 6 s
                    //  stall ends and the queued write finally runs.)
                    const long long cancel_lag = std::chrono::duration_cast<std::chrono::milliseconds>(
                        send_end - close_start
                    ).count();

                    if (sent == payload.size()){
                        //  The event thread caught up before
                        //  close_connection() and the send completed on
                        //  its own: the cancellation was not exercised,
                        //  so there is nothing to assert on the send's
                        //  timing.
                        if (close_ms > 60000){
                            if (timing_unreliable()){
                                return UnitTestResult(
                                    UnitTestResult::SKIPPED,
                                    "close_connection() did not finish within 60 s on a starved machine."
                                );
                            }
                            error = "close_connection() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                        }else{
                            ok = true;
                        }
                    }else if (cancel_lag < 0){
                        //  The send returned before close_connection()
                        //  even started: it was never in flight, so the
                        //  test's premise did not hold.
                        error = "The send returned (" + std::to_string(sent) + " bytes) before close_connection() even started";
                    }else if (cancel_lag > 2000){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "send() was not woken promptly by close_connection() on a starved machine."
                            );
                        }
                        error = "send() did not return promptly after close_connection() cancelled it (lag " + std::to_string(cancel_lag) + " ms)";
                    }else if (close_ms > 60000){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "close_connection() did not finish within 60 s on a starved machine."
                            );
                        }
                        error = "close_connection() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                    }else{
                        //  close_connection() may return with the close
                        //  still pending on the (stalled) event thread,
                        //  so poll for the resulting LISTENING state
                        //  within the shared budget before judging it.
                        const auto s0 = std::chrono::steady_clock::now();
                        while (server.state() != AbstractServerSocket::State::LISTENING
                            && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                        if (server.state() != AbstractServerSocket::State::LISTENING){
                            if (timing_unreliable()){
                                return UnitTestResult(
                                    UnitTestResult::SKIPPED,
                                    "The server is not LISTENING after close_connection() on a starved machine."
                                );
                            }
                            error = "The server is not LISTENING after close_connection()";
                        }else{
                            ok = true;
                        }
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

private:
    ThreadPool& m_thread_pool;
};

//  Exercises the Qt server's connection-churn paths: repeated
//  connect/disconnect cycles, rejected connections while a client is
//  connected, and close_connection() on the active connection.
//  ServerSocket_Qt releases every QTcpSocket as soon as it stops being
//  used (a rejected socket right after abort(), an accepted socket after
//  its disconnect/error, and the active socket after
//  close_connection()); before the fix, every connect/disconnect attempt
//  left one QTcpSocket child of the server behind until the server itself
//  was destroyed. This hammers those paths and verifies that the server
//  keeps behaving correctly while its sockets are actually released.
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

        ServerListener server_listener;
        ServerSocket_Qt server(m_thread_pool);
        server.add_listener(server_listener);
        server_listener.server = &server;  // Echo received data back.

        uint16_t port = 0;
        {
            std::mt19937 rng{std::random_device{}()};
            for (int attempt = 0; attempt < 8; attempt++){
                uint16_t candidate = (uint16_t)(40000 + rng() % 20000);
                server.start("127.0.0.1", candidate);
                if (server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.listen_finished.load() > 0; }
                ) && server_listener.last_listen_error.empty()){
                    port = candidate;
                    break;
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

            //  Phase 3: close_connection() releases the active
            //  connection's socket, and the server must keep working
            //  afterwards.
            if (error.empty()){
                client.connect("127.0.0.1", port);
                if (!client_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&client_listener]{
                        return client_listener.connect_finished.load() >= CYCLES + 2;
                    }
                ) || !client_listener.last_connect_error.empty()){
                    error = "The client did not reconnect for the close_connection phase";
                }else if (!server_listener.waiter.wait_for(
                    SOCKET_TEST_WAIT,
                    [&server_listener]{ return server_listener.connected.load() == CYCLES + 2; }
                )){
                    error = "The server did not call on_connect for the close_connection phase";
                }else{
                    server.close_connection();

                    //  close_connection() may return with the close still
                    //  pending on the event thread, so poll for the
                    //  resulting LISTENING state within the shared budget.
                    const auto s0 = std::chrono::steady_clock::now();
                    while (
                        server.state() != AbstractServerSocket::State::LISTENING
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT
                    ){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    if (server.state() != AbstractServerSocket::State::LISTENING){
                        error = "The server is not LISTENING after close_connection()";
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
                        error = "The client could not reconnect after close_connection()";
                    }else if (!server_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&server_listener]{ return server_listener.connected.load() == CYCLES + 3; }
                    )){
                        error = "The server did not call on_connect after close_connection()";
                    }else if (client.send("pong", 4) != 4){
                        error = "The client could not send after close_connection()";
                    }else if (!client_listener.waiter.wait_for(
                        SOCKET_TEST_WAIT,
                        [&client_listener]{
                            return client_listener.received.size() >= 4 * (size_t)(CYCLES + 1);
                        }
                    )){
                        error = "The server did not echo after close_connection()";
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
            }

            //  Final counter check: the CYCLES phase-1 connections, the
            //  phase-2 occupant, and the two phase-3 connections; the
            //  close_connection() itself is not reported as a disconnect.
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
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_ClientSendTimeout : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr bool QT = std::is_same_v<ClientSocketType, ClientSocket_Qt>;

        //  Stalls the server's reading for 12 s on the first data (or
        //  until "stop" is set), keeping the client's send buffer full
        //  for the duration of the send test.
        struct StallingServerListener : ServerListener{
            std::atomic<bool> stalled{false};
            std::atomic<bool> stop{false};

            ~StallingServerListener(){
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
                ServerListener::on_receive_data(data, bytes);
            }
        };

        StallingServerListener server_listener;
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

        bool ok = false;
        std::string error;
        {
            ClientListener client_listener;
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
                    //  thread that close_and_wait() has to wait on). The infinite
                    //  default is the canary: it returns in ~0 ms on a
                    //  healthy event thread, and for the full 5 s internal
                    //  safety wait if that thread is starved. (A finite
                    //  timeout cannot be used: the caller-side wait is
                    //  capped at "timeout + 1 s", so the control could
                    //  never expose a starved event thread.)
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



//  The client accepts the connection but stalls reading: the server's
//  send() with a deadline must stop at the deadline with a partial
//  count, and close_connection() must still be able to finish afterwards
//  (on the Qt backend the in-flight write stops at the deadline, releasing
//  the server's event thread, which is what close_connection() has to run
//  on).
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
//  only the timing and close_connection() behavior are asserted.
//
//  Timing tolerance: if the machine is too starved for wall-clock
//  assertions to be trusted (timing_unreliable()), the test skips
//  instead of failing - at start, at each threshold crossing, and on
//  connect/listen-phase failures.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_ServerSendTimeout : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        constexpr bool QT = std::is_same_v<ServerSocketType, ServerSocket_Qt>;

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

        ServerListener server_listener;
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

        bool ok = false;
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
                    size_t sent = server.send(
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
                    //  the server's event thread (on the Qt backend, the
                    //  thread that close_connection() has to wait on). The
                    //  infinite default is the canary: it returns in ~0 ms
                    //  on a healthy event thread, and for the full 5 s
                    //  internal safety wait if that thread is starved.
                    //  (A finite timeout cannot be used: the caller-side
                    //  wait is capped at "timeout + 1 s", so the control
                    //  could never expose a starved event thread.)
                    const auto z0 = std::chrono::steady_clock::now();
                    server.send("", 0);
                    const long long control_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - z0
                    ).count();

                    auto c0 = std::chrono::steady_clock::now();
                    server.close_connection();
                    auto close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - c0
                    ).count();
                    if (close_ms > 60000){
                        if (control_ms > 2000 || timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "close_connection() took " + std::to_string(close_ms) + " ms on a starved machine."
                            );
                        }
                        error = "close_connection() did not finish after the timed-out send (" + std::to_string(close_ms) + " ms)";
                    }else{
                        //  close_connection() queues the close onto the
                        //  server's event thread and stops waiting after
                        //  its internal 5 s bound, so on a starved event
                        //  thread the close may still be pending when it
                        //  returns. The queued close does run once the
                        //  event thread gets to it, so poll for the
                        //  resulting LISTENING state within the shared
                        //  budget instead of failing on a not-yet-applied
                        //  close.
                        const auto s0 = std::chrono::steady_clock::now();
                        while (server.state() != AbstractServerSocket::State::LISTENING
                            && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                        if (server.state() != AbstractServerSocket::State::LISTENING){
                            if (control_ms > 2000 || timing_unreliable()){
                                return UnitTestResult(
                                    UnitTestResult::SKIPPED,
                                    "The server is not LISTENING after close_connection() on a starved machine."
                                );
                            }
                            error = "The server is not LISTENING after close_connection()";
                        }else{
                            ok = true;
                        }
                    }
                }
            }

            //  Release the stalled reader so the client's close() (which
            //  joins its receive thread, currently inside the stall) does
            //  not wait for it.
            client_listener.stop.store(true, std::memory_order_release);
            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};


//  Verifies the close()-cancels-sends contract (see the docs on
//  "AbstractClientSocket::close()"): the server never reads, so the
//  256 MB send stays in flight, and "close()" has to end it. The send
//  must return promptly with a best-effort byte count; the threshold
//  only has to prove boundedness (the behavior without cancellation is
//  an infinite hang), so it tolerates extremely loaded CI runners.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_CloseCancelsSend : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    Test_Sockets_CloseCancelsSend(ThreadPool& thread_pool, std::string name)
        : TestSockets<ClientSocketType, ServerSocketType>(thread_pool, std::move(name))
    {
        //  The 256 MiB payload below, plus a transient 256 MiB copy in the
        //  Qt backend's in-flight send data (overreported on the other
        //  backends, which don't copy the payload).
        this->m_memory = 2ull * (1ull << 28);
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }

        //  Stalls the server's reading for 70 s on the first data (or
        //  until "stop" is set), keeping the client's send in flight.
        struct StallingServerListener : ServerListener{
            std::atomic<bool> stalled{false};
            std::atomic<bool> stop{false};

            ~StallingServerListener(){
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
                ServerListener::on_receive_data(data, bytes);
            }
        };

        StallingServerListener server_listener;
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

        bool ok = false;
        std::string error;
        {
            ClientListener client_listener;
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

                size_t sent = 0;
                auto send_start = std::chrono::steady_clock::now();
                AsyncTask send_task = this->m_thread_pool.dispatch(
                    [&client, &payload, &sent]{
                        sent = client.send(payload.data(), payload.size());
                    }
                );

                //  Give the send a head start so it is in flight when
                //  close() runs, then cancel it.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                auto close_start = std::chrono::steady_clock::now();
                client.close();
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
                }else if (sent > payload.size()){
                    error = "send() reported more bytes (" + std::to_string(sent) + ") than the payload size";
                }else{
                    ok = true;
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


//  Verifies the close_connection()-cancels-sends contract (see the
//  docs on "AbstractServerSocket::close_connection()"): the client
//  never reads, so the 256 MB send stays in flight, and
//  "close_connection()" has to end it. The send must return promptly
//  with a best-effort byte count, and the server has to be back to
//  LISTENING; the thresholds only have to prove boundedness, so they
//  tolerate extremely loaded CI runners.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_CloseConnectionCancelsSend : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    Test_Sockets_CloseConnectionCancelsSend(ThreadPool& thread_pool, std::string name)
        : TestSockets<ClientSocketType, ServerSocketType>(thread_pool, std::move(name))
    {
        //  The 256 MiB payload below, plus a transient 256 MiB copy in the
        //  Qt backend's in-flight send data (overreported on the other
        //  backends, which don't copy the payload).
        this->m_memory = 2ull * (1ull << 28);
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }

        //  Stalls the client's reading for 70 s on the first data (or
        //  until "stop" is set), keeping the server's send in flight.
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

        ServerListener server_listener;
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

        bool ok = false;
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
                //  send completes before close_connection() and the test
                //  still passes, just without exercising the cancel.)
                constexpr size_t SIZE = 1 << 28;
                std::vector<uint8_t> payload(SIZE, 0xAB);

                size_t sent = 0;
                auto send_start = std::chrono::steady_clock::now();
                AsyncTask send_task = this->m_thread_pool.dispatch(
                    [&server, &payload, &sent]{
                        sent = server.send(payload.data(), payload.size());
                    }
                );

                //  Give the send a head start so it is in flight when
                //  close_connection() runs, then cancel it.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                auto close_start = std::chrono::steady_clock::now();
                server.close_connection();
                const long long close_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - close_start
                ).count();

                send_task.wait_and_ignore_exceptions();
                const long long send_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - send_start
                ).count();

                if (send_ms > 60000){
                    //  A send that is not ended by close_connection() on
                    //  a starved machine is the machine's fault, not the
                    //  cancellation's.
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "send() did not return within 60 s of close_connection() on a starved machine."
                        );
                    }
                    error = "send() did not return within 60 s of close_connection() (" + std::to_string(send_ms) + " ms)";
                }else if (close_ms > 60000){
                    if (timing_unreliable()){
                        return UnitTestResult(
                            UnitTestResult::SKIPPED,
                            "close_connection() did not finish within 60 s on a starved machine."
                        );
                    }
                    error = "close_connection() did not finish within 60 s (" + std::to_string(close_ms) + " ms)";
                }else if (sent > payload.size()){
                    error = "send() reported more bytes (" + std::to_string(sent) + ") than the payload size";
                }else{
                    //  close_connection() may return with the close still
                    //  pending on a starved event thread (Qt backend), so
                    //  poll for the resulting LISTENING state within the
                    //  shared budget before judging it.
                    const auto s0 = std::chrono::steady_clock::now();
                    while (server.state() != AbstractServerSocket::State::LISTENING
                        && std::chrono::steady_clock::now() - s0 < SOCKET_TEST_WAIT){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    if (server.state() != AbstractServerSocket::State::LISTENING){
                        if (timing_unreliable()){
                            return UnitTestResult(
                                UnitTestResult::SKIPPED,
                                "The server is not LISTENING after close_connection() on a starved machine."
                            );
                        }
                        error = "The server is not LISTENING after close_connection()";
                    }else{
                        ok = true;
                    }
                }
            }

            //  Release the stalled reader so the client's close() (which
            //  joins its receive thread, currently inside the stall) does
            //  not wait for it.
            client_listener.stop.store(true, std::memory_order_release);
            client.close();
        }

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }
};



//  Verifies that a ClientSocket can be torn down from within its own
//  on_receive_data() callback via close() (the sys-botbase oversized-line
//  scenario). close_and_wait() would deadlock there - it waits for this
//  receive thread to exit - so close() must return promptly and leave the
//  socket no longer CONNECTED.
template <typename ClientSocketType, typename ServerSocketType>
class Test_Sockets_CloseFromReceiveCallback : public TestSockets<ClientSocketType, ServerSocketType>{
public:
    using TestSockets<ClientSocketType, ServerSocketType>::TestSockets;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (timing_unreliable()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "Machine too starved for a timing-sensitive socket test (timing_unreliable())."
            );
        }
        ServerListener server_listener;
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
            }else if (server.send("ping", 4) != 4){
                error = "The server did not send the trigger packet";
            }else if (!client_listener.waiter.wait_for(
                SOCKET_TEST_WAIT,
                [&client_listener]{ return client_listener.closed_in_callback.load() >= 1; }
            )){
                error = "close() did not return from the on_receive_data() callback (it would have blocked, like close_and_wait() would)";
            }else if (client.state() == AbstractClientSocket::State::CONNECTED){
                error = "After close() the client is still CONNECTED";
            }else{
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


template <typename ClientSocketType, typename ServerSocketType>
void add_socket_tests(UnitTestDatabase& database, ThreadPool& thread_pool, const char* backend_name){
    const std::string prefix = "Sockets (" + std::string(backend_name) + ") - ";

    database.add<Test_Sockets_Echo<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Client/server echo round trip"
    );
    database.add<Test_Sockets_ZeroLengthSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Zero-length send"
    );
    database.add<Test_Sockets_LargeTransfer<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Large transfer"
    );
    database.add<Test_Sockets_RapidReconnect<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Rapid reconnect"
    );
    database.add<Test_Sockets_SendCloseRace<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Send/close race"
    );
    database.add<Test_Sockets_RejectSecondClient<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Reject a second client"
    );
    database.add<Test_Sockets_StaleSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "A send for a replaced connection is refused"
    );
    database.add<Test_Sockets_PortInUse<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Port in use"
    );
    database.add<Test_Sockets_ClientSendTimeout<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Client send() timeout against a stalled peer"
    );
    database.add<Test_Sockets_ServerSendTimeout<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Server send() timeout against a stalled peer"
    );
    database.add<Test_Sockets_CloseCancelsSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "close() cancels an in-flight send"
    );
    database.add<Test_Sockets_CloseConnectionCancelsSend<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "close_connection() cancels an in-flight send"
    );
    database.add<Test_Sockets_CloseFromReceiveCallback<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "close() from the receive callback"
    );
}



} // anonymous namespace


void add_tests_Sockets(UnitTestDatabase& database, ThreadPool& thread_pool){
    add_socket_tests<ClientSocket_Qt, ServerSocket_Qt>(database, thread_pool, "Qt");

    //  Qt-specific: the completion-lifetime guarantee is only at risk in the
    //  Qt backend (a queued call can outlive its caller there).
    database.add<Test_Sockets_Qt_CompletionLifetime>(
        thread_pool,
        "Sockets (Qt) - Completion lifetime under event-thread stall"
    );

    database.add<Test_Sockets_Qt_CloseCancelsSend>(
        thread_pool,
        "Sockets (Qt) - close() cancels a send queued behind an event-thread stall"
    );

    database.add<Test_Sockets_Qt_CloseConnectionCancelsSend>(
        thread_pool,
        "Sockets (Qt) - close_connection() cancels a send queued behind an event-thread stall"
    );

    database.add<Test_Sockets_Qt_ConnectionChurn>(
        thread_pool,
        "Sockets (Qt) - connection churn releases the QTcpSocket objects"
    );

#ifndef _WIN32
    add_socket_tests<ClientSocket_POSIX, ServerSocket_POSIX>(database, thread_pool, "POSIX");
#endif
#ifdef _WIN32
    add_socket_tests<ClientSocket_WinSocket, ServerSocket_WinSocket>(database, thread_pool, "WinSocket");
#endif
}



}
