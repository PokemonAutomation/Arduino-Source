/*  Sockets Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Loopback tests that combine the generic ClientSocket and ServerSocket
 *  against each other on 127.0.0.1, using whichever backends the generic
 *  classes map to on the current platform.
 *
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/Cpp/Sockets/ClientSocket.h"
#include "Common/Cpp/Sockets/ServerSocket.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Sockets_Tests.h"

namespace PokemonAutomation{

namespace{


struct Waiter{
    std::mutex lock;
    std::condition_variable cv;

    bool wait_for(std::chrono::milliseconds timeout, const std::function<bool()>& pred){
        std::unique_lock<std::mutex> lg(lock);
        return cv.wait_for(lg, timeout, pred);
    }

    void notify(){
        std::lock_guard<std::mutex> lg(lock);
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


class TestSockets : public UnitTest{
public:
    TestSockets(ThreadPool& thread_pool, std::string name)
        : UnitTest("Sockets - " + std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }


protected:
    bool start_server(ServerSocket& server, ServerListener& listener, uint16_t& port) const{
        for (int attempt = 0; attempt < 8; attempt++){
            uint16_t candidate;
            {
                std::lock_guard<std::mutex> lg(m_rng_lock);
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
        ClientSocket& client,
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
    mutable std::mutex m_rng_lock;
    mutable std::mt19937 m_rng{std::random_device{}()};
};


//  Connect, send a packet, verify the echo, disconnect, and check that the
//  server observed both the connect and the disconnect.
class Test_Sockets_Echo : public TestSockets{
public:
    Test_Sockets_Echo(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Client/server echo round trip")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocket client(m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!connect_client(client, client_listener, port, 1)){
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
class Test_Sockets_ZeroLengthSend : public TestSockets{
public:
    Test_Sockets_ZeroLengthSend(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Zero-length send")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocket client(m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!connect_client(client, client_listener, port, 1)){
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
class Test_Sockets_LargeTransfer : public TestSockets{
public:
    Test_Sockets_LargeTransfer(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Large transfer")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE);
        for (size_t i = 0; i < SIZE; i++){
            payload[i] = (uint8_t)(i % 251);
        }

        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocket client(m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!connect_client(client, client_listener, port, 1)){
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
                [&server_listener, SIZE]{ return server_listener.bytes_received.load() >= SIZE; }
            )){
                error = "The server did not receive all the data";
            }else if (!client_listener.waiter.wait_for(
                std::chrono::seconds(30),
                [&client_listener, SIZE]{ return client_listener.received.size() >= SIZE; }
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
class Test_Sockets_RapidReconnect : public TestSockets{
public:
    Test_Sockets_RapidReconnect(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Rapid reconnect")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        constexpr int CYCLES = 50;

        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        size_t expected_echo = 0;
        {
            ClientSocket client(m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            for (int cycle = 0; cycle < CYCLES; cycle++){
                int cycle_no = cycle + 1;

                if (!connect_client(client, client_listener, port, cycle_no)){
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
class Test_Sockets_SendCloseRace : public TestSockets{
public:
    Test_Sockets_SendCloseRace(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Send/close race")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        constexpr size_t SIZE = 1 << 20;
        std::vector<uint8_t> payload(SIZE, 0xAB);

        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocket client(m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!connect_client(client, client_listener, port, 1)){
                error = "The client did not finish connecting";
            }else if (!server_listener.waiter.wait_for(
                std::chrono::seconds(5),
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                std::atomic<size_t> bytes_sent{0};
                AsyncTask send_task = m_thread_pool.dispatch([&client, &payload, &bytes_sent]{
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
                    if (!connect_client(client, client_listener, port, 2)){
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
class Test_Sockets_RejectSecondClient : public TestSockets{
public:
    Test_Sockets_RejectSecondClient(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Reject a second client")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        bool ok = false;
        std::string error;
        {
            ClientSocket client_a(m_thread_pool);
            ClientListener client_a_listener;
            client_a.add_listener(client_a_listener);

            ClientSocket client_b(m_thread_pool);
            ClientListener client_b_listener;
            client_b.add_listener(client_b_listener);

            if (!connect_client(client_a, client_a_listener, port, 1)){
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
class Test_Sockets_PortInUse : public TestSockets{
public:
    Test_Sockets_PortInUse(ThreadPool& thread_pool)
        : TestSockets(thread_pool, "Port in use")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ServerSocket server(m_thread_pool);
        ServerListener server_listener;
        server_listener.server = &server;
        server.add_listener(server_listener);

        uint16_t port;
        if (!start_server(server, server_listener, port)){
            return "Failed to start the server";
        }

        ServerSocket second_server(m_thread_pool);
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
            ClientSocket client(m_thread_pool);
            ClientListener client_listener;
            client.add_listener(client_listener);

            if (!connect_client(client, client_listener, port, 1)){
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



} // anonymous namespace


void add_tests_Sockets(UnitTestDatabase& database, ThreadPool& thread_pool){
    database.add<Test_Sockets_Echo>(thread_pool);
    database.add<Test_Sockets_ZeroLengthSend>(thread_pool);
    database.add<Test_Sockets_LargeTransfer>(thread_pool);
    database.add<Test_Sockets_RapidReconnect>(thread_pool);
    database.add<Test_Sockets_SendCloseRace>(thread_pool);
    database.add<Test_Sockets_RejectSecondClient>(thread_pool);
    database.add<Test_Sockets_PortInUse>(thread_pool);
}



}
