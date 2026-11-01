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
#include <vector>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/Cpp/Sockets/ClientSocket_Qt.h"
#include "Common/Cpp/Sockets/ServerSocket_Qt.h"
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
        for (int attempt = 0; attempt < 8; attempt++){
            uint16_t candidate;
            {
                std::lock_guard<Mutex> lg(m_rng_lock);
                candidate = (uint16_t)(40000 + m_rng() % 20000);
            }
            server.start("127.0.0.1", candidate);
            if (!listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&listener]{ return listener.listen_finished.load() > 0; }
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
            std::chrono::seconds(5),
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
        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocketType client(this->m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!client_listener.last_connect_error.empty()){
                error = "The client failed to connect: " + client_listener.last_connect_error;
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else if (client.send("hello world", 11) != 11){
                error = "The client did not send all 11 bytes";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&server_listener]{ return server_listener.bytes_received.load() >= 11; }
            )){
                error = "The server did not receive the data";
            }else if (!client_listener.waiter.wait_for(
                std::chrono::seconds(5),
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
            std::chrono::seconds(5),
            [&server_listener]{ return server_listener.disconnected.load() >= 1; }
        )){
            error = "The server did not call on_disconnect";
        }

        server.close();

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
        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocketType client(this->m_thread_pool);
            ClientListener client_listener;
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
                    std::chrono::seconds(5),
                    [&server_listener]{ return server_listener.bytes_received.load() >= 4; }
                )){
                    error = "The server did not receive the data";
                }else if (!client_listener.waiter.wait_for(
                    std::chrono::seconds(5),
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

        server.close();

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
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE);
        for (size_t i = 0; i < SIZE; i++){
            payload[i] = (uint8_t)(i % 251);
        }

        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocketType client(this->m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
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

        server.close();

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
        constexpr int CYCLES = 50;

        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        size_t expected_echo = 0;
        {
            ClientSocketType client(this->m_thread_pool);
            ClientListener client_listener;
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
                    std::chrono::seconds(5),
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
                    std::chrono::seconds(5),
                    [&client_listener, expected_echo]{ return client_listener.received.size() >= expected_echo; }
                )){
                    error = "The client did not receive the echo (cycle " + std::to_string(cycle_no) + ")";
                    break;
                }

                client.close();
                if (!server_listener.waiter.wait_for(
                    std::chrono::seconds(5),
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

        server.close();

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
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE, 0xAB);

        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocketType client(this->m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
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

                if (server.state() != AbstractServerSocket::State::LISTENING){
                    error = "The server is not listening after close_connection()";
                }else if (!client_listener.waiter.wait_for(
                    std::chrono::seconds(5),
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
                        std::chrono::seconds(5),
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

        server.close();

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
        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocketType client_a(this->m_thread_pool);
            ClientListener client_a_listener;
            client_a.add_listener(client_a_listener);

            ClientSocketType client_b(this->m_thread_pool);
            ClientListener client_b_listener;
            client_b.add_listener(client_b_listener);

            if (!this->connect_client(client_a, client_a_listener, port, 1)){
                error = "Client A did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                client_b.connect("127.0.0.1", port);
                if (!server_listener.waiter.wait_for(
                    std::chrono::seconds(5),
                    [&server_listener]{ return server_listener.rejected.load() >= 1; }
                )){
                    error = "The server did not reject the second client";
                }else{
                    //  The first client must keep working.
                    if (client_a.send("still here?", 11) != 11){
                        error = "Client A did not send all 11 bytes";
                    }else if (!client_a_listener.waiter.wait_for(
                        std::chrono::seconds(5),
                        [&client_a_listener]{ return client_a_listener.received.size() >= 11; }
                    )){
                        error = "Client A did not receive the echo";
                    }else if (std::memcmp(client_a_listener.received.data(), "still here?", 11) != 0){
                        error = "The echo data was corrupted";
                    }else{
                        //  Client B must have got a final answer to its
                        //  connect attempt, whatever it was.
                        if (!client_b_listener.waiter.wait_for(
                            std::chrono::seconds(5),
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

        server.close();

        if (!ok && !error.empty()){
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
        ServerSocketType server(this->m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!this->start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        ServerSocketType second_server(this->m_thread_pool);
        ServerListener second_listener;
        second_server.add_listener(second_listener);
        second_server.start("127.0.0.1", port);
        if (!second_listener.waiter.wait_for(
            std::chrono::seconds(5),
            [&second_listener]{ return second_listener.listen_finished.load() > 0; }
        )){
            server.close();
            return "The second server did not report its start";
        }
        if (second_listener.last_listen_error.empty()){
            server.close();
            second_server.close();
            return "The second server succeeded on a port that is already in use";
        }
        if (second_server.state() != AbstractServerSocket::State::NOT_RUNNING){
            server.close();
            second_server.close();
            return "The second server is not NOT_RUNNING after a failed start";
        }

        //  The original server must still work.
        bool ok = false;
        {
            ClientSocketType client(this->m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!this->connect_client(client, client_listener, port, 1)){
                ok = false;
            }else if (client.send("ok", 2) != 2){
                ok = false;
            }else if (!client_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&client_listener]{ return client_listener.received.size() >= 2; }
            )){
                ok = false;
            }else{
                ok = true;
            }

            client.close();
        }

        server.close();
        second_server.close();

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
        //  Blocks the event thread while it receives data.
        struct StallListener : AbstractServerSocket::Listener{
            virtual void on_receive_data(const void*, size_t) override{
                std::this_thread::sleep_for(std::chrono::seconds(6));
            }
        };

        ServerSocket_Qt server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);
        StallListener stall_listener;
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
            ClientSocket_Qt client(m_thread_pool);
            ClientListener client_listener;
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

        server.close();

        if (!ok && !error.empty()){
            return error;
        }
        return true;
    }

private:
    ThreadPool& m_thread_pool;
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
    database.add<Test_Sockets_PortInUse<ClientSocketType, ServerSocketType>>(
        thread_pool, prefix + "Port in use"
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

#ifdef _WIN32
    add_socket_tests<ClientSocket_WinSocket, ServerSocket_WinSocket>(database, thread_pool, "WinSocket");
#endif
}



}
