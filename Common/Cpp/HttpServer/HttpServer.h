/*  HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A minimal HTTP/1.1 server built on top of "AbstractServerSocket", aimed
 *  at a single local client that issues one request at a time (for example,
 *  an MCP client talking to the AI Agent Server).
 *
 *  Design notes:
 *  - One connection at a time: the underlying server socket handles one
 *    connection at a time, and a second client gets "on_connect_rejected".
 *  - Requests are processed strictly in order. Bytes of a pipelined request
 *    simply wait in the buffer until the previous response has been sent.
 *  - The user "Handler" may block for a long time; it runs on a thread from
 *    the given thread pool. Poll "HandlerContext::connection_alive()"
 *    between long-running steps to bail out early. Once it returns false,
 *    the response you set is discarded.
 *  - No TLS. Request bodies: "Content-Length" only (chunked is rejected
 *    with 411). Response bodies: always "Content-Length" (no chunked).
 *    No upgrades.
 *
 *  Threading:
 *  - All "Listener" callbacks run on threads from the thread pool, never
 *    on the calling thread.
 *  - Do not call "start()" or "stop()" from within a "Listener" callback:
 *    "stop()" joins the very pool threads that fire the callbacks.
 *  - The thread pool should be an unlimited, long-lived one (for example
 *    "GlobalThreadPools::unlimited_normal()"): the server keeps an accept
 *    thread and a per-connection thread alive for as long as it runs.
 *
 */

#ifndef PokemonAutomation_HttpServer_H
#define PokemonAutomation_HttpServer_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Sockets/AbstractServerSocket.h"

namespace PokemonAutomation{

class ThreadPool;



class HttpServer : private AbstractServerSocket::Listener{
public:
    struct Request{
        std::string method;                    //  e.g. "POST"
        std::string path;                      //  e.g. "/mcp" (without the query)
        std::string query;                     //  the part after '?', if any
        std::map<std::string, std::string> headers;   //  names are lower-cased
        std::string body;

        //  "name" is lower-cased first; returns "" if the header is absent.
        const char* header(const std::string& name) const;
    };

    struct Response{
        int status = 200;
        std::string reason;    //  empty = the standard phrase for "status"
        std::vector<std::pair<std::string, std::string>> headers;
        std::string body;

        //  End the connection after this response, even on HTTP/1.1.
        bool connection_close = false;
    };

    //  Passed to the handler for the duration of the call.
    struct HandlerContext{
        //  False once the connection this request came in on is closed or
        //  the server is stopping. Once false, the response is discarded
        //  even if you set one.
        bool connection_alive() const;

    private:
        explicit HandlerContext(const std::atomic<bool>& p_dead)
            : m_dead(p_dead)
        {}

        const std::atomic<bool>& m_dead;

        friend class HttpServer;
    };

    //  Runs on a thread from the thread pool, never on the socket's
    //  connection thread. It may block (for example, while a tool runs on
    //  the program thread), but it must eventually return: "stop()" blocks
    //  until the in-flight handler returns. It must not throw; exceptions
    //  are turned into a 500 response.
    using Handler = std::function<void(const Request& request, Response& response, HandlerContext& context)>;

    struct Listener{
        //  Called after "start()" finishes. If successful, "error_message"
        //  is empty. Otherwise it contains the error message and the server
        //  is not listening; call "stop()" to clean up before retrying.
        virtual void on_listen_finished(const std::string& error_message){}

        //  Called when a client connects.
        virtual void on_connect(){}

        //  Called when a second client tries to connect while one is
        //  already connected; that client is immediately closed.
        virtual void on_connect_rejected(){}

        //  Called when the client disconnects (a server-side "stop()" does
        //  not report a disconnect).
        virtual void on_disconnect(const std::string& error_message){}
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }


public:
    explicit HttpServer(ThreadPool& thread_pool);
    ~HttpServer();

    //  Starts listening. Asynchronous: "Listener::on_listen_finished" is
    //  called on a thread from the thread pool with the result. "handler"
    //  receives every fully parsed request; if it is null, all requests
    //  get a 500. Returns without starting if the server is already
    //  started.
    void start(const std::string& address, uint16_t port, Handler handler);

    //  Stops the server and blocks until it is fully down: the active
    //  connection is closed, the in-flight handler (if any) is waited for,
    //  and the socket's worker threads are joined. Afterwards "start()"
    //  can be called again.
    void stop();

    //  True if "start()" has been called and the server has not been
    //  stopped again.
    bool is_running() const;


private:
    //  AbstractServerSocket::Listener.
    virtual void on_listen_finished(const std::string& error_message) override;
    virtual void on_connect() override;
    virtual void on_connect_rejected() override;
    virtual void on_receive_data(const void* data, size_t bytes) override;
    virtual void on_disconnect(const std::string& error_message) override;

    //  One request that is waiting to be parsed out of the buffer.
    enum class ParseState{
        NEED_MORE,
        READY,
        ERROR,
    };

    struct ParsedRequest{
        ParseState state = ParseState::NEED_MORE;

        //  ERROR
        int error_status = 400;
        std::string error_reason;
        std::vector<std::pair<std::string, std::string>> error_headers;

        //  READY
        Request request;
        bool http_1_1 = true;
        bool connection_close_requested = false;
        bool expect_continue = false;
    };

    //  Per-connection state. The connection task is the only thread that
    //  parses or writes; "on_receive_data"/"on_disconnect" only append data
    //  and flip flags.
    struct Connection{
        Mutex lock;
        ConditionVariable cv;
        std::string buffer;
        std::atomic<bool> dead{false};
    };

    //  Runs on a thread from the thread pool, one instance per connection.
    void connection_task(const std::shared_ptr<Connection>& conn, AbstractServerSocket* socket);

    //  Parses one complete request out of "buffer", removing it on success.
    //  Must be called with "conn->lock" held.
    static ParsedRequest try_parse(std::string& buffer);

    //  Marks the connection dead and closes it at the socket level. No-op
    //  if the socket is no longer connected to this client.
    void end_connection(const std::shared_ptr<Connection>& conn, AbstractServerSocket* socket);

    //  Writes "bytes" to the connection completely; returns false (and
    //  ends the connection) if the peer went away or the write stalled.
    bool write_raw(const std::shared_ptr<Connection>& conn, AbstractServerSocket* socket, const std::string& bytes);

    //  Formats and writes the final response.
    bool write_response(
        const std::shared_ptr<Connection>& conn,
        AbstractServerSocket* socket,
        const Response& response,
        bool keep_alive
    );

    //  The standard reason phrase for "status", or "Unknown Status".
    static const char* status_reason(int status);

    //  Builds the wire format of a response.
    static std::string format_response(const Response& response, bool keep_alive);

    ThreadPool& m_thread_pool;
    ListenerSet<Listener> m_listeners;

    //  Guards m_socket, m_connection, m_connection_task and m_handler.
    //  Never held while waiting on a condition variable. Mutable so that
    //  "is_running()" can take it from a const method.
    mutable Mutex m_lock;
    std::unique_ptr<AbstractServerSocket> m_socket;
    std::shared_ptr<Connection> m_connection;
    AsyncTask m_connection_task;
    Handler m_handler;

    //  Set by "stop()"; checked by the connection tasks between requests
    //  and by "HandlerContext".
    std::atomic<bool> m_stop_requested{false};
};



}
#endif
