/*  Server Socket 2 Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  API-shape tests for the new AbstractServerSocket2 /
 *  AbstractServerSocketConnection pair, written against an in-memory
 *  stub backend. They document the intended contracts (pointer
 *  lifetime, inert-after-close, on_disconnect() only for peer
 *  closes, one connection at a time) and pin the API while it is
 *  under review; the real backends land separately.
 *
 */

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Sockets/AbstractServerSocket2.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "ServerSocket2_Tests.h"

namespace PokemonAutomation{

namespace{


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
    Test_ServerSocket2_StubApi(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
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


private:
    ThreadPool& m_thread_pool;
};



}


void add_tests_ServerSocket2(UnitTestDatabase& database, ThreadPool& thread_pool){
    database.add<Test_ServerSocket2_StubApi>(
        thread_pool,
        "ServerSocket2 (Stub) - API shape and lifetime contracts"
    );
}


}
