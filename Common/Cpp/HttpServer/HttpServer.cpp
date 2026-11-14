/*  HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implementation notes:
 *  - The socket's receive/disconnect callbacks only append data to the
 *    per-connection buffer and flip flags. One "connection task" per
 *    connection (from the thread pool) does all parsing and writing, so
 *    the handler may run for a long time without stalling the socket's
 *    receive loop.
 *  - Every write goes through "AbstractServerSocket::send()" with the
 *    connection's socket id (captured at accept time), so a response
 *    that outlives its connection - the peer disconnected and the next
 *    client was accepted in the meantime - is refused by the socket
 *    instead of reaching the new client's stream.
 *  - "Expect: 100-continue" is answered as soon as the headers are
 *    validated, while the body is still pending (a waiting client is
 *    blocked on that interim response).
 *  - Task handles live in "m_tasks" under a dedicated "m_task_lock"
 *    (the socket library's "m_task_lock" pattern): destroying an
 *    "AsyncTask" handle blocks until that task finishes, and the task's
 *    final cleanup takes "m_lock", so a handle must never be destroyed
 *    while holding "m_lock". "on_connect()" may therefore find the
 *    previous connection's task still finishing (for example, still in a
 *    long handler call); its handle is kept and reaped later.
 *  - "stop()" marks the active connection dead before it waits for the
 *    in-flight task: the socket suppresses its disconnect callback for a
 *    close_connection() close, so without the mark a handler polling
 *    "connection_alive()" would never see the shutdown. "stop()" then
 *    moves the socket out under "m_lock" and destroys it after releasing
 *    the lock: "~ServerSocket" joins the socket's worker threads, and
 *    their callbacks take "m_lock".
 *
 */

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/Cpp/HttpServer/HttpServer.h"
#include "Common/Cpp/Sockets/ServerSocket.h"

namespace PokemonAutomation{

namespace{


constexpr size_t MAX_REQUEST_LINE_BYTES = 8192;
constexpr size_t MAX_HEADER_SECTION_BYTES = 65536;
constexpr size_t MAX_HEADER_COUNT = 100;
constexpr size_t MAX_BODY_BYTES = 1u << 20;      //  1 MiB
//  The total per-connection receive buffer; enforced when data is
//  appended, not at parse time.
constexpr size_t MAX_BUFFER_BYTES = 2u << 20;

//  A stalled peer is declared dead after this; loopback writes of a few MB
//  finish in milliseconds, so this is very generous.
constexpr std::chrono::milliseconds WRITE_TIMEOUT{30000};


bool is_token_char(char c){
    return std::isalnum((unsigned char)c) != 0
        || std::strchr("-!#$%&'*+.^_`|~", c) != nullptr;
}


std::string to_lower(std::string value){
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
        return (char)std::tolower(c);
    });
    return value;
}


//  Strips the leading and trailing spaces and tabs (OWS).
std::string trim_ows(std::string value){
    static const char OWS[] = " \t";
    const auto begin = value.find_first_not_of(OWS);
    if (begin == std::string::npos){
        return "";
    }
    const auto end = value.find_last_not_of(OWS);
    return value.substr(begin, end - begin + 1);
}


//  The concrete HandlerContext: "connection_alive()" is simply the
//  connection's dead flag.
struct HandlerContextImpl : AbstractHttpServer::HandlerContext{
    explicit HandlerContextImpl(const std::atomic<bool>& dead)
        : m_dead(dead)
    {}

    virtual bool connection_alive() const override{
        return !m_dead.load(std::memory_order_acquire);
    }

private:
    const std::atomic<bool>& m_dead;
};



} // anonymous namespace



const char* HttpServer::status_reason(int status){
    switch (status){
    case 100:
        return "Continue";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 304:
        return "Not Modified";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 408:
        return "Request Timeout";
    case 411:
        return "Length Required";
    case 413:
        return "Payload Too Large";
    case 426:
        return "Upgrade Required";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 503:
        return "Service Unavailable";
    case 505:
        return "Version Not Supported";
    default:
        return "Unknown Status";
    }
}


std::string HttpServer::format_response(const Response& response, bool keep_alive){
    std::string out;
    out += "HTTP/1.1 ";
    out += std::to_string(response.status);
    out += " ";
    const char* reason = response.reason.empty() ? status_reason(response.status) : response.reason.c_str();
    out += reason;
    out += "\r\n";

    for (const auto& header : response.headers){
        //  The server sets both of these below.
        if (to_lower(header.first) == "content-length" ||
            to_lower(header.first) == "connection"
        ){
            continue;
        }
        out += header.first;
        out += ": ";
        out += header.second;
        out += "\r\n";
    }

    out += "Content-Length: ";
    out += std::to_string(response.body.size());
    out += "\r\n";
    out += "Connection: ";
    out += keep_alive ? "keep-alive" : "close";
    out += "\r\n\r\n";
    out += response.body;

    return out;
}



HttpServer::HttpServer(ThreadPool& thread_pool)
    : m_thread_pool(thread_pool)
{}


HttpServer::~HttpServer(){
    stop();
}


void HttpServer::start(const std::string& address, uint16_t port, Handler handler){
    AbstractServerSocket* socket = nullptr;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_socket != nullptr){
            return;
        }
        m_handler = std::move(handler);
        m_stop_requested.store(false, std::memory_order_release);
        m_socket = std::make_unique<ServerSocket>(m_thread_pool);
        //  Register before start(): the accept thread is dispatched by
        //  start() and may report on_listen_finished() as soon as it runs.
        m_socket->add_listener(*this);
        socket = m_socket.get();
    }

    //  Asynchronous: on_listen_finished() fires on a thread from the pool.
    socket->start(address, port);
}


void HttpServer::stop(){
    m_stop_requested.store(true, std::memory_order_release);

    AbstractServerSocket* socket = nullptr;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_socket.get();
    }
    if (socket == nullptr){
        return;
    }

    //  Mark the active connection dead: the socket suppresses its
    //  disconnect callback for a close_connection() close, so without
    //  this a handler polling connection_alive() would never see the
    //  shutdown, and the wait for it below would never end.
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_connection != nullptr){
            m_connection->dead.store(true, std::memory_order_release);
            m_connection->cv.notify_all();
        }
    }

    //  Kill the active connection, if any: it cancels in-flight writes and
    //  makes the socket's connection thread exit. The socket keeps
    //  listening until it is destroyed below, so it may accept a few more
    //  clients in the meantime; on_connect() sees m_stop_requested and
    //  closes them without dispatching a connection task.
    socket->close_connection();

    //  Wait for the in-flight connection tasks. The handles are moved out
    //  under m_task_lock (never destroyed while holding m_lock: a task's
    //  final cleanup takes m_lock, and destroying a handle blocks until
    //  the task finishes). Loop, because a task may have been dispatched
    //  between the previous ones being joined and the socket being
    //  destroyed.
    while (true){
        std::vector<AsyncTask> tasks;
        {
            std::lock_guard<Mutex> lg(m_task_lock);
            tasks.swap(m_tasks);
        }
        if (tasks.empty()){
            break;
        }
        for (auto& task : tasks){
            task.wait_and_ignore_exceptions();
        }
    }

    //  Take the socket out under m_lock and destroy it after releasing
    //  the lock: "~ServerSocket" joins the socket's worker threads, and
    //  their callbacks (on_connect and friends) take m_lock.
    std::unique_ptr<AbstractServerSocket> owned;
    {
        std::lock_guard<Mutex> lg(m_lock);
        owned = std::move(m_socket);
        m_connection = nullptr;
        m_handler = Handler();
    }
}


bool HttpServer::is_running() const{
    std::lock_guard<Mutex> lg(m_lock);
    return (m_socket != nullptr);
}



void HttpServer::on_listen_finished(const std::string& error_message){
    m_listeners.run_method(&Listener::on_listen_finished, error_message);
}


void HttpServer::on_connect(){
    AbstractServerSocket* socket = nullptr;
    std::shared_ptr<Connection> conn;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_socket.get();
        if (socket != nullptr &&
            !m_stop_requested.load(std::memory_order_acquire)
        ){
            conn = std::make_shared<Connection>();
            //  The previous socket connection is over (the socket only
            //  accepts a new client once the previous one is gone), but
            //  its task may still be finishing (for example, still inside
            //  a long handler call), and the socket's on_disconnect
            //  callback for it may not have run yet: it runs after the
            //  socket has already moved back to listening. Mark it dead
            //  now, before the new connection takes over the socket's
            //  active fd, so the old task discards its response instead
            //  of delivering it to the new client.
            if (m_connection != nullptr){
                m_connection->dead.store(true, std::memory_order_release);
                m_connection->cv.notify_all();
            }
            m_connection = conn;
            //  The token of the connection this one will serve; send()
            //  refuses to write through a socket that has moved on.
            conn->socket_id = socket->connection_id();
        }
    }
    if (conn == nullptr){
        //  Stopping (or the socket is gone): close the connection without
        //  a connection task.
        if (socket != nullptr){
            socket->close_connection();
        }
    }else{
        //  Dispatch outside m_lock: the task's final cleanup takes
        //  m_lock, and so does stop(). The task handles live in m_tasks
        //  under a dedicated m_task_lock (the socket library's
        //  m_task_lock pattern): destroying an AsyncTask handle blocks
        //  until its task finishes, so a handle must never be destroyed
        //  while holding m_lock.
        try{
            AsyncTask task = m_thread_pool.dispatch(
                [this, conn, socket]{
                    connection_task(conn, socket);
                }
            );
            std::lock_guard<Mutex> lg(m_task_lock);
            //  Reap finished handles (destroying one that is already
            //  finished is a no-op). Keep unfinished ones: the
            //  previous connection's task may still be finishing up
            //  (for example, still inside a long handler call) by the
            //  time this connection is accepted; it holds only its
            //  own (dead) connection.
            m_tasks.erase(
                std::remove_if(m_tasks.begin(), m_tasks.end(), [](const AsyncTask& task){
                    return task.is_finished();
                }),
                m_tasks.end()
            );
            m_tasks.push_back(std::move(task));
        }catch (...){
            //  The task could not be started: drop the connection and
            //  keep listening.
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_connection.get() == conn.get()){
                    m_connection = nullptr;
                }
            }
            socket->close_connection();
        }
    }
    m_listeners.run_method(&Listener::on_connect);
}


void HttpServer::on_connect_rejected(){
    m_listeners.run_method(&Listener::on_connect_rejected);
}


void HttpServer::on_receive_data(const void* data, size_t bytes){
    std::shared_ptr<Connection> conn;
    AbstractServerSocket* socket = nullptr;
    {
        std::lock_guard<Mutex> lg(m_lock);
        conn = m_connection;
        socket = m_socket.get();
    }
    if (conn == nullptr){
        return;
    }
    bool exceeded = false;
    {
        std::lock_guard<Mutex> lg(conn->lock);
        exceeded = conn->buffer.size() + bytes > MAX_BUFFER_BYTES;
        if (!exceeded){
            const char* ptr = (const char*)data;
            conn->buffer.append(ptr, bytes);
        }
    }
    conn->cv.notify_all();
    if (exceeded && socket != nullptr){
        //  The stream is longer than we are willing to buffer. It is
        //  likely in the middle of a request, so there is no reliable
        //  response left to send: just end the connection. ("socket" can
        //  be null only while stop() is tearing the socket down, in which
        //  case there is no connection task left to wake up.)
        end_connection(conn, socket);
    }
}


void HttpServer::on_disconnect(const std::string& error_message){
    std::shared_ptr<Connection> conn;
    {
        std::lock_guard<Mutex> lg(m_lock);
        //  The socket may already be serving the next client by the time
        //  this callback runs: the socket's connection thread sets the
        //  state back to LISTENING before it reports the disconnect, so a
        //  fast reconnect can be accepted in between. In that case this
        //  disconnect is about the previous connection, which on_connect()
        //  has already marked dead; the current one belongs to the new
        //  client and must not be killed.
        if (m_socket == nullptr ||
            m_socket->state() != AbstractServerSocket::State::CONNECTED
        ){
            conn = m_connection;
        }
    }
    if (conn != nullptr){
        conn->dead.store(true, std::memory_order_release);
        conn->cv.notify_all();
    }
    m_listeners.run_method(&Listener::on_disconnect, error_message);
}



void HttpServer::connection_task(
    const std::shared_ptr<Connection>& conn,
    AbstractServerSocket* socket
){
    //  The handler may be replaced or cleared by start()/stop() while this
    //  task is waiting; a copy taken under m_lock is stable for the task's
    //  whole lifetime, because stop() waits for the task before clearing it.
    Handler handler;
    {
        std::lock_guard<Mutex> lg(m_lock);
        handler = m_handler;
    }

    //  "Expect: 100-continue" is answered at most once, and only while
    //  the body is still pending.
    bool sent_continue = false;

    while (true){
        if (m_stop_requested.load(std::memory_order_acquire) ||
            conn->dead.load(std::memory_order_acquire)
        ){
            break;
        }

        ParsedRequest parsed;
        size_t buffer_size;
        {
            std::lock_guard<Mutex> lg(conn->lock);
            parsed = try_parse(conn->buffer);
            buffer_size = conn->buffer.size();
        }

        if (parsed.state == ParseState::NEED_BODY){
            //  The headers are complete and valid, but the body has not
            //  fully arrived. A client that sent "Expect: 100-continue" is
            //  still waiting for this before sending the body: answer it
            //  now, not after the (still missing) body arrives. (If the
            //  whole request was already buffered, the state is READY and
            //  the final response alone moves the client on; RFC 9110).
            if (parsed.expect_continue && !sent_continue){
                static const char CONTINUE_RESPONSE[] = "HTTP/1.1 100 Continue\r\n\r\n";
                if (!write_raw(conn, socket, CONTINUE_RESPONSE)){
                    break;
                }
                sent_continue = true;
            }
        }

        if (parsed.state == ParseState::NEED_MORE ||
            parsed.state == ParseState::NEED_BODY
        ){
            //  Wait for the rest of this request, a disconnect, or a
            //  stop. The predicate requires the buffer to have grown past
            //  what the parse just saw: with a partial request,
            //  "!buffer.empty()" alone is permanently true and would spin
            //  this loop at 100% CPU until the request completes.
            std::unique_lock<Mutex> lg(conn->lock);
            conn->cv.wait_for(lg, std::chrono::milliseconds(10), [this, &conn, buffer_size]{
                return conn->buffer.size() != buffer_size
                    || conn->dead.load(std::memory_order_relaxed)
                    || m_stop_requested.load(std::memory_order_relaxed);
            });
            continue;
        }

        if (parsed.state == ParseState::ERROR_RESPONSE){
            Response response;
            response.status = parsed.error_status;
            response.headers = std::move(parsed.error_headers);
            response.body = parsed.error_reason + "\n";
            write_response(conn, socket, response, false);
            end_connection(conn, socket);
            break;
        }

        //  A complete request is ready to be handled.
        Response response;
        {
            HandlerContextImpl context(conn->dead);
            try{
                if (handler){
                    handler(parsed.request, response, context);
                }else{
                    response.status = 500;
                    response.body = "No handler is registered.\n";
                    response.connection_close = true;
                }
            }catch (...){
                response = Response();
                response.status = 500;
                response.body = "The handler threw an exception.\n";
                response.connection_close = true;
            }
        }

        //  The connection may have closed or been replaced by a new
        //  client while the handler was running; discard the response in
        //  that case.
        if (conn->dead.load(std::memory_order_acquire) ||
            m_stop_requested.load(std::memory_order_acquire)
        ){
            break;
        }

        const bool keep_alive = parsed.http_1_1
            && !parsed.connection_close_requested
            && !response.connection_close;
        if (!write_response(conn, socket, response, keep_alive)){
            break;
        }
        if (!keep_alive){
            end_connection(conn, socket);
            break;
        }
        //  Keep-alive: the next request (possibly already buffered) is
        //  picked up at the top of the loop.
    }

    conn->dead.store(true, std::memory_order_release);
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_connection.get() == conn.get()){
            m_connection = nullptr;
        }
    }
}


HttpServer::ParsedRequest HttpServer::try_parse(std::string& buffer){
    ParsedRequest result;

    auto error = [&result](int status, const char* reason, std::vector<std::pair<std::string, std::string>> headers = {}){
        result.state = ParseState::ERROR_RESPONSE;
        result.error_status = status;
        result.error_reason = reason;
        result.error_headers = std::move(headers);
    };

    //  The header section ends at the blank line. "header_end" points at
    //  the "\r\n" that terminates the last header (or the request line
    //  when there are no headers).
    const size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos){
        if (buffer.size() > MAX_HEADER_SECTION_BYTES){
            error(400, "Malformed request.");
        }
        return result;   //  NEED_MORE: the headers have not fully arrived yet.
    }
    //  The same limit applies to a complete header section: the
    //  in-progress check above only fires while the terminator is still
    //  missing, so a large section that arrives in one piece would
    //  otherwise sail through.
    if (header_end + 4 > MAX_HEADER_SECTION_BYTES){
        error(400, "Malformed request.");
        return result;
    }

    const size_t line_end = buffer.find("\r\n");
    if (line_end == std::string::npos){
        error(400, "Malformed request.");
        return result;
    }
    if (line_end > MAX_REQUEST_LINE_BYTES){
        error(400, "Request line too long.");
        return result;
    }

    //  "METHOD SP TARGET SP VERSION"
    const std::string line = buffer.substr(0, line_end);
    const size_t sp1 = line.find(' ');
    if (sp1 == std::string::npos || sp1 == 0){
        error(400, "Malformed request.");
        return result;
    }
    std::string method = line.substr(0, sp1);
    for (char c : method){
        if (c < 'A' || c > 'Z'){
            error(400, "Malformed request.");
            return result;
        }
    }
    const size_t sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos){
        error(400, "Malformed request.");
        return result;
    }
    std::string target = line.substr(sp1 + 1, sp2 - sp1 - 1);
    if (target.empty()){
        error(400, "Malformed request.");
        return result;
    }
    for (char c : target){
        if (c == ' ' || c == '\r' || c == '\n' || c == '\0'){
            error(400, "Malformed request.");
            return result;
        }
    }
    const std::string version = line.substr(sp2 + 1);
    const bool http_1_1 = (version == "HTTP/1.1");
    const bool http_1_0 = (version == "HTTP/1.0");
    if (!http_1_1 && !http_1_0){
        error(505, "HTTP version not supported.");
        return result;
    }

    //  Headers: [line_end + 2, header_end)
    const size_t section_begin = line_end + 2;
    std::string section;
    if (header_end >= section_begin){
        section = buffer.substr(section_begin, header_end - section_begin);
    }
    size_t header_count = 0;
    size_t pos = 0;
    while (pos < section.size()){
        const size_t eol = section.find("\r\n", pos);
        const size_t length = (eol == std::string::npos) ? section.size() - pos : eol - pos;
        const std::string header_line = section.substr(pos, length);

        if (header_line.empty() || ++header_count > MAX_HEADER_COUNT){
            error(400, "Malformed request.");
            return result;
        }

        const size_t colon = header_line.find(':');
        if (colon == std::string::npos || colon == 0){
            error(400, "Malformed request.");
            return result;
        }
        std::string name = header_line.substr(0, colon);
        for (char c : name){
            if (!is_token_char(c)){
                error(400, "Malformed request.");
                return result;
            }
        }
        std::string value = trim_ows(header_line.substr(colon + 1));
        for (char c : value){
            if ((unsigned char)c < 0x20 || (unsigned char)c == 0x7F){
                error(400, "Malformed request.");
                return result;
            }
        }
        name = to_lower(std::move(name));
        if (name == "content-length" && result.request.headers.count("content-length") != 0){
            error(400, "Duplicate content-length.");
            return result;
        }
        result.request.headers[std::move(name)] = std::move(value);

        if (eol == std::string::npos){
            break;
        }
        pos = eol + 2;
    }

    //  Special headers.
    if (result.request.headers.count("transfer-encoding") != 0){
        //  "Not Implemented" rather than "Length Required": chunked
        //  encoding is a (unsupported) representation of the body, not a
        //  missing length (RFC 9112).
        error(501, "Chunked transfer encoding is not supported.");
        return result;
    }

    uint64_t content_length = 0;
    {
        const auto content_length_it = result.request.headers.find("content-length");
        if (content_length_it != result.request.headers.end()){
            const std::string& value = content_length_it->second;
            if (value.empty() || value.size() > 10){
                error(400, "Malformed content-length.");
                return result;
            }
            for (char c : value){
                if (c < '0' || c > '9'){
                    error(400, "Malformed content-length.");
                    return result;
                }
            }
            content_length = std::stoull(value);
            if (content_length > MAX_BODY_BYTES){
                error(413, "Request body too large.");
                return result;
            }
        }
    }

    const auto connection_it = result.request.headers.find("connection");
    if (connection_it != result.request.headers.end() &&
        to_lower(connection_it->second).find("close") != std::string::npos
    ){
        result.connection_close_requested = true;
    }

    const auto expect_it = result.request.headers.find("expect");
    if (expect_it != result.request.headers.end() &&
        to_lower(expect_it->second).find("100-continue") != std::string::npos
    ){
        result.expect_continue = true;
    }

    const size_t consumed = header_end + 4 + (size_t)content_length;
    if (buffer.size() < consumed){
        //  NEED_BODY: the headers are complete and valid, the body has
        //  not fully arrived yet. In this state the buffer holds nothing
        //  but the bytes of this one request ("consumed" is at most a
        //  header section plus the body cap), so the buffer cannot exceed
        //  MAX_BUFFER_BYTES here; the overall cap is enforced when data
        //  is appended instead.
        result.state = ParseState::NEED_BODY;
        return result;
    }

    result.state = ParseState::READY;
    result.request.method = std::move(method);
    const size_t query = target.find('?');
    result.request.path = target.substr(0, query);
    if (query != std::string::npos){
        result.request.query = target.substr(query + 1);
    }
    result.request.body = buffer.substr(header_end + 4, content_length);
    buffer.erase(0, consumed);
    result.http_1_1 = http_1_1;
    return result;
}


void HttpServer::end_connection(
    const std::shared_ptr<Connection>& conn,
    AbstractServerSocket* socket
){
    conn->dead.store(true, std::memory_order_release);
    conn->cv.notify_all();
    //  Close the socket's connection only while it still belongs to this
    //  connection: by the time a task notices its own connection died,
    //  the socket may already be serving the next client, and closing
    //  unconditionally would kill that client's connection instead.
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_connection.get() != conn.get()){
            return;
        }
    }
    socket->close_connection();
}


bool HttpServer::write_raw(
    const std::shared_ptr<Connection>& conn,
    AbstractServerSocket* socket,
    const std::string& bytes
){
    //  "conn->socket_id" is the socket's connection token when this
    //  connection was accepted: if the socket has moved on to the next
    //  client since then, the send is refused instead of being delivered
    //  into the new client's socket.
    const size_t sent = socket->send(bytes.data(), bytes.size(), WRITE_TIMEOUT, conn->socket_id);
    if (sent != bytes.size()){
        //  The peer went away, or the write stalled: the connection is
        //  unusable either way.
        end_connection(conn, socket);
        return false;
    }
    return true;
}


bool HttpServer::write_response(
    const std::shared_ptr<Connection>& conn,
    AbstractServerSocket* socket,
    const Response& response,
    bool keep_alive
){
    return write_raw(conn, socket, format_response(response, keep_alive));
}



}
