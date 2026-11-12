/*  HTTP Server Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Loopback tests that run a real "HttpServer" (the platform's default
 *  server socket backend) on 127.0.0.1 and drive it with raw
 *  "ClientSocket" clients.
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
#include "Common/Cpp/HttpServer/HttpServer.h"
#include "Common/Cpp/Sockets/ClientSocket.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "HttpServer_Tests.h"

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


//  Long enough that a moderately loaded machine can make it, short enough
//  that a real regression is still reported within the CI timeouts.
constexpr std::chrono::seconds HTTP_TEST_WAIT{60};


//  A handler gate: handlers notify "started" when they begin, and block on
//  "wait_for_release()" until the test releases them.
struct HandlerGate{
    Waiter started;

    Mutex lock;
    ConditionVariable cv;
    std::atomic<bool> released{false};

    void wait_for_release(std::chrono::milliseconds timeout){
        std::unique_lock<Mutex> lg(lock);
        cv.wait_for(lg, timeout, [this]{
            return released.load(std::memory_order_relaxed);
        });
    }

    void release(){
        released.store(true, std::memory_order_release);
        std::lock_guard<Mutex> lg(lock);
        cv.notify_all();
    }
};


//  A raw TCP client that accumulates everything it receives.
//  "received" and the error strings are protected by "waiter.lock": the
//  socket's receive thread writes them, the test thread reads them.
struct TestClient{
    Waiter waiter;
    std::string received;
    std::atomic<int> connected{0};
    std::atomic<int> disconnected{0};
    std::string connect_error;
    std::string disconnect_error;

    struct Listener : AbstractClientSocket::Listener{
        TestClient* client;

        virtual void on_connect_finished(const std::string& error_message) override{
            {
                std::lock_guard<Mutex> lg(client->waiter.lock);
                client->connect_error = error_message;
            }
            client->connected++;
            client->waiter.notify();
        }

        virtual void on_receive_data(const void* data, size_t bytes) override{
            {
                std::lock_guard<Mutex> lg(client->waiter.lock);
                client->received.append((const char*)data, bytes);
            }
            client->waiter.notify();
        }

        virtual void on_disconnect(const std::string& error_message) override{
            {
                std::lock_guard<Mutex> lg(client->waiter.lock);
                client->disconnect_error = error_message;
            }
            client->disconnected++;
            client->waiter.notify();
        }
    } listener_impl;

    ClientSocket socket;

    explicit TestClient(ThreadPool& thread_pool)
        : socket(thread_pool)
    {
        listener_impl.client = this;
        socket.add_listener(listener_impl);
    }

    bool connect(uint16_t port){
        socket.connect("127.0.0.1", port);
        return waiter.wait_for(
            HTTP_TEST_WAIT,
            [this]{ return connected.load() >= 1; }
        );
    }

    bool send_all(const std::string& bytes){
        size_t total = 0;
        while (total < bytes.size()){
            const size_t sent = socket.send(bytes.data() + total, bytes.size() - total);
            if (sent == 0){
                return false;
            }
            total += sent;
        }
        return true;
    }

    bool wait_for_bytes(size_t n){
        //  The predicate runs with "waiter.lock" held, so it may read
        //  "received" directly.
        return waiter.wait_for(
            HTTP_TEST_WAIT,
            [this, n]{ return received.size() >= n; }
        );
    }

    //  A consistent copy of everything received so far, for assertions.
    std::string received_snapshot(){
        std::lock_guard<Mutex> lg(waiter.lock);
        return received;
    }
};


struct TestServerListener : HttpServer::Listener{
    Waiter waiter;
    std::atomic<int> listen_finished{0};
    //  Protected by "waiter.lock" (written by the socket's thread).
    std::string last_listen_error;
    std::atomic<int> connected{0};
    std::atomic<int> rejected{0};
    std::atomic<int> disconnected{0};

    virtual void on_listen_finished(const std::string& error_message) override{
        {
            std::lock_guard<Mutex> lg(waiter.lock);
            last_listen_error = error_message;
        }
        listen_finished++;
        waiter.notify();
    }

    std::string last_listen_error_snapshot(){
        std::lock_guard<Mutex> lg(waiter.lock);
        return last_listen_error;
    }

    virtual void on_connect() override{
        connected++;
        waiter.notify();
    }

    virtual void on_connect_rejected() override{
        rejected++;
        waiter.notify();
    }

    virtual void on_disconnect(const std::string& error_message) override{
        disconnected++;
        waiter.notify();
    }
};


class TestHttpServer : public UnitTest{
public:
    TestHttpServer(ThreadPool& thread_pool, std::string name)
        : UnitTest(std::move(name))
        , m_thread_pool(thread_pool)
    {
        m_threads = 2;
    }


protected:
    //  Starts the server with "handler" on a free loopback port, waiting
    //  for the listen to finish. "handler" is copied on each attempt.
    bool start_server(HttpServer& server, TestServerListener& listener, HttpServer::Handler handler, uint16_t& port) const{
        for (int attempt = 0; attempt < 8; attempt++){
            uint16_t candidate;
            {
                std::lock_guard<Mutex> lg(m_rng_lock);
                candidate = (uint16_t)(40000 + m_rng() % 20000);
            }
            const int finished_before = listener.listen_finished.load();
            server.start("127.0.0.1", candidate, handler);
            if (!listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [finished_before, &listener]{
                    return listener.listen_finished.load() > finished_before;
                }
            )){
                server.stop();
                continue;
            }
            if (listener.last_listen_error_snapshot().empty()){
                port = candidate;
                return true;
            }
            //  A failed listen leaves the server "started but not
            //  listening"; stop() cleans that up before the next attempt.
            server.stop();
        }
        return false;
    }

protected:
    ThreadPool& m_thread_pool;

private:
    mutable Mutex m_rng_lock;
    mutable std::mt19937 m_rng{std::random_device{}()};
};


//  A POST request is parsed (method, path, query, headers, body) and the
//  response carries the extra headers and the exact body; the connection
//  stays keep-alive.
class Test_Http_PostEcho : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        auto handler = [](const HttpServer::Request& request, HttpServer::Response& response, HttpServer::HandlerContext&){
            response.status = 200;
            response.headers.push_back({"X-Test", "yes"});
            response.body = request.path + "|" + request.query + "|" + request.header("X-Client") + "|" + request.body;
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!server_listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else if (!client.send_all(
                "POST /mcp?session=abc HTTP/1.1\r\n"
                "Host: 127.0.0.1\r\n"
                "X-Client: unit-test\r\n"
                "Content-Length: 5\r\n"
                "\r\n"
                "hello"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "X-Test: yes\r\n"
                    "Content-Length: 32\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "/mcp|session=abc|unit-test|hello";
                if (!client.wait_for_bytes(expected.size())){
                    error = "The client did not receive the full response";
                }else if (client.received_snapshot() != expected){
                    error = "Bad response:\n" + client.received_snapshot();
                }
            }
            //  The client goes out of scope here and closes the socket.
        }

        if (error.empty() &&
            !server_listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [&server_listener]{ return server_listener.disconnected.load() >= 1; }
            )
        ){
            error = "The server did not call on_disconnect";
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Two requests sent back to back on one connection (the second is already
//  buffered while the first is being answered): both must be answered, in
//  order.
class Test_Http_PipelinedKeepAlive : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        auto handler = [](const HttpServer::Request& request, HttpServer::Response& response, HttpServer::HandlerContext&){
            response.status = 200;
            response.body = "r" + request.body;
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST /a HTTP/1.1\r\n"
                "Content-Length: 3\r\n"
                "\r\n"
                "one"
                "POST /b HTTP/1.1\r\n"
                "Content-Length: 3\r\n"
                "\r\n"
                "two"
            )){
                error = "The client did not send the requests";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 4\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "rone"
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 4\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "rtwo";
                if (!client.wait_for_bytes(expected.size())){
                    error = "The client did not receive both responses (got " + std::to_string(client.received_snapshot().size()) + " of " + std::to_string(expected.size()) + " bytes)";
                }else if (client.received_snapshot() != expected){
                    error = "Bad response:\n" + client.received_snapshot();
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  "Connection: close" in the request: the response says so, the server
//  closes the connection, and it keeps serving new clients.
class Test_Http_ConnectionClose : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        auto handler = [](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            response.status = 200;
            response.body = "done";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "Connection: close\r\n"
                "\r\n"
                "x"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 4\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "done";
                if (!client.wait_for_bytes(expected.size())){
                    error = "The client did not receive the full response";
                }else if (client.received_snapshot() != expected){
                    error = "Bad response:\n" + client.received_snapshot();
                }else if (!client.waiter.wait_for(
                    HTTP_TEST_WAIT,
                    [&client]{ return client.disconnected.load() >= 1; }
                )){
                    error = "The server did not close the connection";
                }
            }
        }

        if (error.empty()){
            //  The server must keep listening for a new client.
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not reconnect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "y"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 4\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "done";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "The reconnected client did not get a good response:\n" + client.received_snapshot();
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A request line that is not "METHOD SP TARGET SP VERSION" gets a 400 and
//  a closed connection; the server recovers and serves a new client.
class Test_Http_BadRequest : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        auto handler = [](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all("GARBAGE\r\n\r\n")){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Length: 19\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "Malformed request.\n";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "Bad error response:\n" + client.received_snapshot();
                }
            }
        }

        if (error.empty()){
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not reconnect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 2\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "ok";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "The server did not recover:\n" + client.received_snapshot();
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  Chunked request bodies are not supported: 501 and a closed connection,
//  and the handler is never called.
class Test_Http_ChunkedRejected : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        std::atomic<bool> handler_called{false};
        auto handler = [&handler_called](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            handler_called = true;
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 501 Not Implemented\r\n"
                    "Content-Length: 44\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "Chunked transfer encoding is not supported.\n";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "Bad error response:\n" + client.received_snapshot();
                }
            }
        }

        if (error.empty() && handler_called.load()){
            error = "The handler was called for a rejected request";
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A content-length above the body limit is rejected with 413 before the
//  (never sent) body arrives, and the handler is never called.
class Test_Http_BodyTooLarge : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        std::atomic<bool> handler_called{false};
        auto handler = [&handler_called](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            handler_called = true;
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 2097152\r\n"
                "\r\n"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 413 Payload Too Large\r\n"
                    "Content-Length: 24\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "Request body too large.\n";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "Bad error response:\n" + client.received_snapshot();
                }
            }
        }

        if (error.empty() && handler_called.load()){
            error = "The handler was called for a rejected request";
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A complete header section over the size limit is rejected with 400,
//  even though it arrives in one piece (the in-progress limit check only
//  fires while the terminator is still missing).
class Test_Http_HeadersTooLarge : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        std::atomic<bool> handler_called{false};
        auto handler = [&handler_called](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            handler_called = true;
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        //  50 headers of about 1.5 KiB each: about 75 KiB of headers -
        //  under the 100-header count limit, over the 64 KiB size limit.
        std::string request = "POST / HTTP/1.1\r\n";
        for (int i = 0; i < 50; i++){
            request += "X-Filler-";
            request += std::to_string(i);
            request += ": ";
            request += std::string(1490, 'a');
            request += "\r\n";
        }
        request += "\r\n";

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(request)){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Length: 19\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "Malformed request.\n";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "Bad error response:\n" + client.received_snapshot();
                }
            }
        }

        if (error.empty() && handler_called.load()){
            error = "The handler was called for a rejected request";
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  A connection that sends more data than the receive buffer cap (about
//  2 MiB) is closed without a response, and the server keeps listening.
class Test_Http_BufferCap : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        std::atomic<int> handler_calls{0};
        auto handler = [&handler_calls](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            handler_calls++;
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else{
                //  A never-completing request far larger than the buffer
                //  cap: the server closes the connection without a
                //  response. (The send may fail partway through once the
                //  server has hung up; that is expected.)
                const std::string flood(3u << 20, 'A');
                client.send_all(flood);
                if (!client.waiter.wait_for(
                    HTTP_TEST_WAIT,
                    [&client]{ return client.disconnected.load() >= 1; }
                )){
                    error = "The server did not close the over-cap connection";
                }
            }
        }

        if (error.empty() && handler_calls.load() != 0){
            error = "The handler was called for a capped connection";
        }

        if (error.empty()){
            //  The server must still be listening afterwards.
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect after the cap test";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 2\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "ok";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "The server did not answer after the cap test:\n" + client.received_snapshot();
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  "Expect: 100-continue" is answered with "100 Continue" as soon as the
//  headers are validated - before the body arrives, which is what a
//  client waiting for the interim response is waiting for - and the
//  final response carries the body.
class Test_Http_ExpectContinue : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        auto handler = [](const HttpServer::Request& request, HttpServer::Response& response, HttpServer::HandlerContext&){
            response.status = 200;
            response.body = request.body;
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 5\r\n"
                "Expect: 100-continue\r\n"
                "\r\n"
            )){
                error = "The client did not send the headers";
            }else{
                const std::string continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
                if (!client.wait_for_bytes(continue_response.size())
                    || client.received_snapshot().compare(0, continue_response.size(), continue_response) != 0
                ){
                    error = "The server did not answer the interim response:\n" + client.received_snapshot();
                }else if (!client.send_all("hello")){
                    error = "The client did not send the body";
                }else{
                    const std::string expected =
                        "HTTP/1.1 100 Continue\r\n\r\n"
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 5\r\n"
                        "Connection: keep-alive\r\n"
                        "\r\n"
                        "hello";
                    if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                        error = "Bad response:\n" + client.received_snapshot();
                    }
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  The client disconnects while the handler is still running: the handler
//  sees it via "HandlerContext::connection_alive()", its response is
//  discarded, and the server keeps serving new clients.
class Test_Http_DisconnectDuringHandler : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        HandlerGate gate;
        std::atomic<int> call_count{0};
        std::atomic<bool> first_call_alive{false};
        std::atomic<bool> first_call_saw_disconnect{false};

        auto handler = [&gate, &call_count, &first_call_alive, &first_call_saw_disconnect](
            const HttpServer::Request&,
            HttpServer::Response& response,
            HttpServer::HandlerContext& context
        ){
            const int call = call_count.fetch_add(1) + 1;
            if (call == 1){
                first_call_alive = context.connection_alive();
                gate.started.notify();
                gate.wait_for_release(HTTP_TEST_WAIT);
                first_call_saw_disconnect = !context.connection_alive();
            }
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "x"
            )){
                error = "The client did not send the request";
            }else if (!gate.started.wait_for(
                HTTP_TEST_WAIT,
                [&first_call_alive]{ return first_call_alive.load(); }
            )){
                error = "The handler did not start";
            }
            //  The client goes out of scope here and closes the socket,
            //  while the handler is still blocked on the gate.
        }

        if (error.empty() &&
            !server_listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [&server_listener]{ return server_listener.disconnected.load() >= 1; }
            )
        ){
            error = "The server did not call on_disconnect";
        }

        if (error.empty()){
            gate.release();
        }

        if (error.empty()){
            //  The server must keep serving new clients.
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not reconnect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "y"
            )){
                error = "The client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 2\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "ok";
                if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                    error = "The server did not recover:\n" + client.received_snapshot();
                }
            }
        }

        if (error.empty()){
            //  Wait for the first handler to have recorded its observation.
            if (!gate.started.wait_for(
                HTTP_TEST_WAIT,
                [&first_call_saw_disconnect, &call_count]{
                    return call_count.load() >= 1 && first_call_saw_disconnect.load();
                }
            )){
                error = "The handler did not record the disconnect";
            }
        }

        if (error.empty()){
            if (!first_call_alive.load()){
                error = "The handler did not see the connection as alive on entry";
            }else if (!first_call_saw_disconnect.load()){
                error = "The handler did not see the disconnect via connection_alive()";
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  The client disconnects while the handler is still running, and a new
//  client connects before the handler returns: the old task must discard
//  its response instead of writing it into the new client's socket.
class Test_Http_ReconnectWhileHandlerRunning : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        HandlerGate gate;
        std::atomic<bool> first_call_alive{false};

        auto handler = [&gate, &first_call_alive](
            const HttpServer::Request& request,
            HttpServer::Response& response,
            HttpServer::HandlerContext& context
        ){
            if (request.path == "/a"){
                first_call_alive = context.connection_alive();
                gate.started.notify();
                gate.wait_for_release(HTTP_TEST_WAIT);
            }
            response.status = 200;
            response.body = (request.path == "/a") ? "A" : "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        TestClient client_a(m_thread_pool);
        TestClient client_b(m_thread_pool);
        {
            if (!client_a.connect(port)){
                error = "The first client did not connect";
            }else if (!client_a.send_all(
                "POST /a HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "x"
            )){
                error = "The first client did not send the request";
            }else if (!gate.started.wait_for(
                HTTP_TEST_WAIT,
                [&first_call_alive]{ return first_call_alive.load(); }
            )){
                error = "The handler did not start";
            }
        }

        if (error.empty()){
            //  Disconnect the first client (closing its socket while it is
            //  still in scope, so its connection task keeps running) and
            //  wait for the server to report the disconnect.
            client_a.socket.close();
            if (!server_listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [&server_listener]{ return server_listener.disconnected.load() >= 1; }
            )){
                error = "The server did not report the first client's disconnect";
            }
        }

        if (error.empty()){
            //  Connect the second client while the first connection's
            //  handler is still blocked on the gate. Waiting for the
            //  second on_connect guarantees the second connection has
            //  replaced the first one in the server by the time the gate
            //  is released.
            if (!client_b.connect(port)){
                error = "The second client did not connect";
            }else if (!server_listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 2; }
            )){
                error = "The server did not accept the second client";
            }
        }

        if (error.empty()){
            gate.release();
        }

        if (error.empty()){
            //  The second client's first bytes must be its own response:
            //  the first client's late response was discarded, not
            //  written into this socket.
            if (!client_b.send_all(
                "POST /b HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "y"
            )){
                error = "The second client did not send the request";
            }else{
                const std::string expected =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 2\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n"
                    "ok";
                if (!client_b.wait_for_bytes(expected.size()) || client_b.received_snapshot() != expected){
                    error = "The second client received a stray response:\n" + client_b.received_snapshot();
                }
            }
        }

        if (error.empty() && !first_call_alive.load()){
            error = "The handler did not see the connection as alive on entry";
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  "stop()" while the handler is still running blocks until the handler
//  returns, and "start()" works afterwards.
class Test_Http_StopDuringHandler : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        HandlerGate gate;
        std::atomic<bool> handler_started{false};
        std::atomic<bool> saw_disconnect_on_stop{false};

        auto handler = [&gate, &handler_started, &saw_disconnect_on_stop](
            const HttpServer::Request&,
            HttpServer::Response& response,
            HttpServer::HandlerContext& context
        ){
            handler_started = true;
            gate.started.notify();
            gate.wait_for_release(HTTP_TEST_WAIT);
            //  Then stay alive while the connection is alive: the pattern
            //  a long-running tool call uses. stop() marks the connection
            //  dead before it waits for this handler (the socket's
            //  close_connection() close suppresses the disconnect
            //  callback), so the loop must terminate - without that mark
            //  this would spin forever and stop() would hang.
            while (context.connection_alive()){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            saw_disconnect_on_stop = !context.connection_alive();
            response.status = 200;
            response.body = "late";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient client(m_thread_pool);
            if (!client.connect(port)){
                error = "The client did not connect";
            }else if (!client.send_all(
                "POST / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "x"
            )){
                error = "The client did not send the request";
            }else if (!gate.started.wait_for(
                HTTP_TEST_WAIT,
                [&handler_started]{ return handler_started.load(); }
            )){
                error = "The handler did not start";
            }
        }

        if (error.empty()){
            //  stop() blocks until the in-flight handler returns. Release
            //  the handler from a pool thread so this thread can call it.
            //  The handle is kept (dispatch is [[nodiscard]]); its
            //  destructor waits for the trivial release task to finish.
            AsyncTask releaser_task = m_thread_pool.dispatch([&gate]{
                gate.release();
            });
            server.stop();
            if (server.is_running()){
                error = "The server is still running after stop()";
            }
        }

        if (error.empty() && !saw_disconnect_on_stop.load()){
            error = "The handler did not see the connection as gone after stop()";
        }

        if (error.empty()){
            //  Start again (on a fresh port) and serve a fresh request.
            auto simple_handler = [](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
                response.status = 200;
                response.body = "ok";
            };
            if (!start_server(server, server_listener, simple_handler, port)){
                error = "Failed to restart the server";
            }else{
                TestClient client(m_thread_pool);
                if (!client.connect(port)){
                    error = "The client did not connect to the restarted server";
                }else if (!client.send_all(
                    "POST / HTTP/1.1\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n"
                )){
                    error = "The client did not send the request";
                }else{
                    const std::string expected =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 2\r\n"
                        "Connection: keep-alive\r\n"
                        "\r\n"
                        "ok";
                    if (!client.wait_for_bytes(expected.size()) || client.received_snapshot() != expected){
                        error = "The restarted server did not answer:\n" + client.received_snapshot();
                    }
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};


//  While one client is connected, a second one is rejected.
class Test_Http_RejectSecondClient : public TestHttpServer{
public:
    using TestHttpServer::TestHttpServer;

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        TestServerListener server_listener;
        HttpServer server(m_thread_pool);
        server.add_listener(server_listener);

        auto handler = [](const HttpServer::Request&, HttpServer::Response& response, HttpServer::HandlerContext&){
            response.status = 200;
            response.body = "ok";
        };

        uint16_t port;
        if (!start_server(server, server_listener, handler, port)){
            return "Failed to start the server";
        }

        std::string error;
        {
            TestClient first(m_thread_pool);
            if (!first.connect(port)){
                error = "The first client did not connect";
            }else if (!server_listener.waiter.wait_for(
                HTTP_TEST_WAIT,
                [&server_listener]{ return server_listener.connected.load() >= 1; }
            )){
                error = "The server did not call on_connect";
            }else{
                TestClient second(m_thread_pool);
                second.connect(port);   //  May complete at the TCP level; the server closes it.
                if (!server_listener.waiter.wait_for(
                    HTTP_TEST_WAIT,
                    [&server_listener]{ return server_listener.rejected.load() >= 1; }
                )){
                    error = "The server did not call on_connect_rejected";
                }
            }
        }

        server.stop();
        if (!error.empty()){
            return error;
        }
        return true;
    }
};



} // anonymous namespace


void add_tests_HttpServer(UnitTestDatabase& database, ThreadPool& thread_pool){
    database.add<Test_Http_PostEcho>(
        thread_pool, "HttpServer - POST request/response with headers, query and body"
    );
    database.add<Test_Http_PipelinedKeepAlive>(
        thread_pool, "HttpServer - Two pipelined requests on one keep-alive connection"
    );
    database.add<Test_Http_ConnectionClose>(
        thread_pool, "HttpServer - Connection: close and reconnection"
    );
    database.add<Test_Http_BadRequest>(
        thread_pool, "HttpServer - Malformed request line"
    );
    database.add<Test_Http_ChunkedRejected>(
        thread_pool, "HttpServer - Chunked request bodies are rejected"
    );
    database.add<Test_Http_BodyTooLarge>(
        thread_pool, "HttpServer - Oversized content-length is rejected"
    );
    database.add<Test_Http_HeadersTooLarge>(
        thread_pool, "HttpServer - An oversized header section is rejected"
    );
    database.add<Test_Http_BufferCap>(
        thread_pool, "HttpServer - A connection over the buffer cap is closed"
    );
    database.add<Test_Http_ExpectContinue>(
        thread_pool, "HttpServer - Expect: 100-continue"
    );
    database.add<Test_Http_DisconnectDuringHandler>(
        thread_pool, "HttpServer - Disconnect while the handler is running"
    );
    database.add<Test_Http_ReconnectWhileHandlerRunning>(
        thread_pool, "HttpServer - Reconnect while the previous handler is still running"
    );
    database.add<Test_Http_StopDuringHandler>(
        thread_pool, "HttpServer - stop() while the handler is running"
    );
    database.add<Test_Http_RejectSecondClient>(
        thread_pool, "HttpServer - A second client is rejected"
    );
}



}
