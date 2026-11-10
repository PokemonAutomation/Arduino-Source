/*  HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The "AbstractServerSocket" implementation of "AbstractHttpServer": a
 *  minimal HTTP/1.1 server on the platform's default server socket, for a
 *  single local client that issues one request at a time. The request/
 *  response types, the handler contract, and the threading rules are in
 *  "AbstractHttpServer.h"; this file documents the implementation.
 *
 *  Implementation notes:
 *  - The socket's receive/disconnect callbacks only append data to the
 *    per-connection buffer and flip flags. One "connection task" per
 *    connection (from the thread pool) does all parsing and writing, so
 *    the handler may run for a long time without stalling the socket's
 *    receive loop.
 *  - Task handles live in "m_tasks" under a dedicated "m_task_lock"
 *    (the socket library's "m_task_lock" pattern): destroying an
 *    "AsyncTask" handle blocks until that task finishes, and the task's
 *    final cleanup takes "m_lock", so a handle must never be destroyed
 *    while holding "m_lock". "on_connect()" may therefore find the
 *    previous connection's task still finishing (for example, still in a
 *    long handler call); its handle is kept and reaped later.
 *  - "stop()" moves the socket out under "m_lock" and destroys it after
 *    releasing the lock: "~ServerSocket" joins the socket's worker
 *    threads, and their callbacks take "m_lock".
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
    virtual void on_connect() override;
    virtual void on_connect_rejected() override;
    virtual void on_receive_data(const void* data, size_t bytes) override;
    virtual void on_disconnect(const std::string& error_message) override;

    //  One request that is waiting to be parsed out of the buffer.
    enum class ParseState{
        NEED_MORE,
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

        //  READY
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
    };

    //  Runs on a thread from the thread pool, one instance per
    //  connection.
    void connection_task(const std::shared_ptr<Connection>& conn, AbstractServerSocket* socket);

    //  Parses one complete request out of "buffer", removing it on
    //  success. Must be called with "conn->lock" held.
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
