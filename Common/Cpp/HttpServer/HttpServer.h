/*  HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The "AbstractServerSocket" implementation of "AbstractHttpServer": a
 *  minimal HTTP/1.1 server on the platform's default server socket
 *  backend, for a single local client that issues one request at a time.
 *  The request/response types, the handler contract, and the threading
 *  rules are in "AbstractHttpServer.h"; this file documents the
 *  implementation.
 *
 *  The transport's per-connection objects make the connection identity
 *  explicit: the socket's callbacks pass the connection they are about,
 *  and all I/O happens on that connection object. A response that
 *  outlives its connection is sent into an inert shell (send() returns
 *  0) instead of being refused by a connection id, and closing a
 *  connection can never touch a later client's socket.
 *
 */

#ifndef PokemonAutomation_HttpServer_H
#define PokemonAutomation_HttpServer_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/HttpServer/AbstractHttpServer.h"
#include "Common/Cpp/Sockets/AbstractServerSocket.h"

namespace PokemonAutomation{

class ThreadPool;



class HttpServer : public AbstractHttpServer, private AbstractServerSocket::Listener{
public:
    //  The interface's types, re-exported under this class's name.
    using AbstractHttpServer::Request;
    using AbstractHttpServer::Response;
    using AbstractHttpServer::HandlerContext;
    using AbstractHttpServer::Handler;
    using AbstractHttpServer::Listener;


public:
    explicit HttpServer(ThreadPool& thread_pool);
    ~HttpServer();

    virtual void start(const std::string& address, uint16_t port, Handler handler) override;
    virtual void stop() override;
    virtual bool is_running() const override;


private:
    //  AbstractServerSocket::Listener.
    virtual void on_listen_finished(const std::string& error_message) override;
    virtual void on_connect(AbstractServerSocket::ConnectionPtr connection) override;
    virtual void on_connect_rejected() override;
    virtual void on_receive_data(
        AbstractServerSocket::ConnectionPtr connection,
        const void* data,
        size_t bytes
    ) override;
    virtual void on_disconnect(
        AbstractServerSocket::ConnectionPtr connection,
        const std::string& error_message
    ) override;


private:
    //  One request that is waiting to be parsed out of the buffer.
    enum class ParseState{
        NEED_MORE,
        NEED_BODY,
        READY,
        //  Named "ERROR_RESPONSE" rather than "ERROR" because "ERROR" is
        //  a macro on Windows (winerror.h).
        ERROR_RESPONSE,
    };

    struct ParsedRequest{
        ParseState state = ParseState::NEED_MORE;

        //  ERROR_RESPONSE
        int error_status = 400;
        std::string error_reason;
        std::vector<std::pair<std::string, std::string>> error_headers;

        //  READY / NEED_BODY
        Request request;
        bool http_1_1 = true;
        bool connection_close_requested = false;
        bool expect_continue = false;
    };

    //  Per-connection state. The connection task is the only thread that
    //  parses or writes; "on_receive_data"/"on_disconnect" only append
    //  data and flip flags.
    struct Connection{
        Mutex lock;
        ConditionVariable cv;
        std::string buffer;
        std::atomic<bool> dead{false};

        //  The transport. The connection shell stays valid but inert
        //  after it closes, so this reference may be held forever.
        AbstractServerSocket::ConnectionPtr socket;
    };

    //  Runs on a thread from the thread pool, one instance per
    //  connection.
    void connection_task(const std::shared_ptr<Connection>& conn);

    //  Parses one complete request out of "buffer", removing it on
    //  success. Must be called with "conn->lock" held.
    static ParsedRequest try_parse(std::string& buffer);

    //  Marks the connection dead and closes its transport. No-op if the
    //  transport is already closed; it can only ever affect this
    //  connection, never a later client's.
    void end_connection(const std::shared_ptr<Connection>& conn);

    //  Writes "bytes" to the connection completely; returns false (and
    //  ends the connection) if the peer went away or the write stalled.
    bool write_raw(const std::shared_ptr<Connection>& conn, const std::string& bytes);

    //  Formats and writes the final response.
    bool write_response(
        const std::shared_ptr<Connection>& conn,
        const Response& response,
        bool keep_alive
    );

    //  The standard reason phrase for "status", or "Unknown Status".
    static const char* status_reason(int status);

    //  Builds the wire format of a response.
    static std::string format_response(const Response& response, bool keep_alive);


private:
    ThreadPool& m_thread_pool;

    //  Guards m_socket, m_connection, and m_handler. Never held across a
    //  blocking wait, a task destruction, or a socket destruction.
    mutable Mutex m_lock;
    std::unique_ptr<AbstractServerSocket> m_socket;
    std::shared_ptr<Connection> m_connection;
    Handler m_handler;

    //  Guards m_tasks. Never held while an AsyncTask destructor runs:
    //  the destructor blocks until the task finishes, and the task's
    //  final cleanup takes m_lock.
    Mutex m_task_lock;
    std::vector<AsyncTask> m_tasks;

    //  Set by "stop()"; checked by the connection tasks between requests
    //  and by "HandlerContext".
    std::atomic<bool> m_stop_requested{false};
};



}
#endif
