/*  Agent Server: HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <atomic>
#include <cctype>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <QHostAddress>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "AgentServer_HttpServer.h"

namespace PokemonAutomation{
namespace AgentServer{



const std::string& HttpRequest::header(const std::string& name) const{
    static const std::string EMPTY;
    auto iter = headers.find(name);
    return iter == headers.end() ? EMPTY : iter->second;
}

HttpResponse HttpResponse::text(int status, std::string body){
    HttpResponse ret;
    ret.status = status;
    ret.content_type = "text/plain; charset=utf-8";
    ret.body = std::move(body);
    return ret;
}
HttpResponse HttpResponse::json(int status, std::string body){
    HttpResponse ret;
    ret.status = status;
    ret.content_type = "application/json";
    ret.body = std::move(body);
    return ret;
}



namespace{

const size_t MAX_HEADER_BYTES = 32 * 1024;

std::string to_lower(std::string text){
    for (char& ch : text){
        ch = (char)std::tolower((unsigned char)ch);
    }
    return text;
}
std::string trim(const std::string& text){
    size_t start = text.find_first_not_of(" \t");
    if (start == std::string::npos){
        return "";
    }
    size_t end = text.find_last_not_of(" \t");
    return text.substr(start, end - start + 1);
}

HttpParseResult parse_error(int status, std::string message){
    HttpParseResult ret;
    ret.status = HttpParseStatus::ERROR;
    ret.error_status = status;
    ret.error_message = std::move(message);
    return ret;
}

const char* reason_phrase(int status){
    switch (status){
    case 100: return "Continue";
    case 200: return "OK";
    case 202: return "Accepted";
    case 204: return "No Content";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 411: return "Length Required";
    case 413: return "Content Too Large";
    case 415: return "Unsupported Media Type";
    case 431: return "Request Header Fields Too Large";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 503: return "Service Unavailable";
    case 505: return "HTTP Version Not Supported";
    default:  return "Unknown";
    }
}

}



HttpParseResult parse_http_request(std::string& buffer, HttpRequest& request, size_t max_body_bytes){
    size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos){
        if (buffer.size() > MAX_HEADER_BYTES){
            return parse_error(431, "Request headers are too large.");
        }
        return HttpParseResult();
    }
    if (header_end > MAX_HEADER_BYTES){
        return parse_error(431, "Request headers are too large.");
    }

    HttpRequest parsed;

    //  Request line: METHOD SP TARGET SP VERSION
    size_t line_end = buffer.find("\r\n");
    std::string request_line = buffer.substr(0, line_end);
    size_t space0 = request_line.find(' ');
    size_t space1 = space0 == std::string::npos ? std::string::npos : request_line.find(' ', space0 + 1);
    if (space0 == std::string::npos || space1 == std::string::npos){
        return parse_error(400, "Malformed request line.");
    }
    parsed.method = request_line.substr(0, space0);
    std::string target = request_line.substr(space0 + 1, space1 - space0 - 1);
    std::string version = request_line.substr(space1 + 1);
    if (!version.starts_with("HTTP/1.")){
        return parse_error(505, "Only HTTP/1.x is supported.");
    }
    size_t question = target.find('?');
    parsed.path = target.substr(0, question);
    if (question != std::string::npos){
        parsed.query = target.substr(question + 1);
    }

    //  Headers
    size_t pos = line_end + 2;
    while (pos < header_end){
        size_t end = buffer.find("\r\n", pos);
        if (end == std::string::npos || end > header_end){
            end = header_end;
        }
        std::string line = buffer.substr(pos, end - pos);
        pos = end + 2;
        if (line.empty()){
            continue;
        }
        if (line[0] == ' ' || line[0] == '\t'){
            return parse_error(400, "Folded header lines are not supported.");
        }
        size_t colon = line.find(':');
        if (colon == std::string::npos || colon == 0){
            return parse_error(400, "Malformed header line.");
        }
        std::string name = to_lower(trim(line.substr(0, colon)));
        std::string value = trim(line.substr(colon + 1));
        auto iter = parsed.headers.find(name);
        if (iter == parsed.headers.end()){
            parsed.headers.emplace(std::move(name), std::move(value));
        }else{
            iter->second += ", " + value;
        }
    }

    std::string connection = to_lower(parsed.header("connection"));
    if (version == "HTTP/1.0"){
        parsed.keep_alive = connection.find("keep-alive") != std::string::npos;
    }else{
        parsed.keep_alive = connection.find("close") == std::string::npos;
    }

    //  Body
    if (!parsed.header("transfer-encoding").empty()){
        return parse_error(411, "Chunked request bodies are not supported; send Content-Length.");
    }
    size_t body_bytes = 0;
    const std::string& length_text = parsed.header("content-length");
    if (!length_text.empty()){
        if (length_text.size() > 12 || !std::all_of(length_text.begin(), length_text.end(), ::isdigit)){
            return parse_error(400, "Invalid Content-Length.");
        }
        body_bytes = (size_t)std::stoull(length_text);
        if (body_bytes > max_body_bytes){
            return parse_error(413, "Request body is too large.");
        }
    }
    size_t body_start = header_end + 4;
    if (buffer.size() - body_start < body_bytes){
        HttpParseResult ret;
        ret.expect_continue = to_lower(parsed.header("expect")) == "100-continue";
        return ret;
    }

    parsed.body = buffer.substr(body_start, body_bytes);
    buffer.erase(0, body_start + body_bytes);
    request = std::move(parsed);

    HttpParseResult ret;
    ret.status = HttpParseStatus::COMPLETE;
    return ret;
}


std::string serialize_http_response(const HttpResponse& response, bool keep_alive){
    std::string ret = "HTTP/1.1 " + std::to_string(response.status) + " " + reason_phrase(response.status) + "\r\n";
    if (!response.content_type.empty()){
        ret += "Content-Type: " + response.content_type + "\r\n";
    }
    ret += "Content-Length: " + std::to_string(response.body.size()) + "\r\n";
    ret += keep_alive ? "Connection: keep-alive\r\n" : "Connection: close\r\n";
    for (const auto& header : response.headers){
        ret += header.first + ": " + header.second + "\r\n";
    }
    ret += "\r\n";
    ret += response.body;
    return ret;
}




//  One client connection. Only touched on the server thread.
struct HttpConnection{
    QPointer<QTcpSocket> socket;
    std::string buffer;
    bool busy = false;              //  a request is being handled; later bytes wait
    bool sent_continue = false;     //  "100 Continue" already sent for this request
};


struct HttpServer::Internal{
    Internal(Logger& p_logger, HttpHandler p_handler)
        : logger(p_logger)
        , handler(std::move(p_handler))
    {}

    Logger& logger;
    HttpHandler handler;

    QThread thread;
    QObject* context = nullptr;         //  lives on `thread`; target for posted work
    QTcpServer* server = nullptr;       //  lives on `thread`; child of `context`
    std::map<QTcpSocket*, std::shared_ptr<HttpConnection>> connections;    //  `thread` only

    std::atomic<bool> listening{false};
    std::atomic<uint16_t> port{0};

    //  Worker threads post their responses through `context` while holding this
    //  lock, and `stop()` clears `context` under it, so no worker posts to a
    //  deleted object.
    std::mutex post_lock;
    QObject* post_target = nullptr;

    //  Count of handlers still running, so `stop()` can wait for them.
    std::mutex inflight_lock;
    std::condition_variable inflight_cv;
    size_t inflight = 0;


    //  ---- Everything below runs on `thread`. ----

    void on_new_connection(){
        while (QTcpSocket* socket = server->nextPendingConnection()){
            auto connection = std::make_shared<HttpConnection>();
            connection->socket = socket;
            connections[socket] = connection;
            QObject::connect(socket, &QTcpSocket::readyRead, context, [this, socket]{
                on_ready_read(socket);
            });
            QObject::connect(socket, &QTcpSocket::disconnected, context, [this, socket]{
                connections.erase(socket);
                socket->deleteLater();
            });
        }
    }

    void on_ready_read(QTcpSocket* socket){
        auto iter = connections.find(socket);
        if (iter == connections.end()){
            return;
        }
        QByteArray data = socket->readAll();
        iter->second->buffer.append(data.constData(), (size_t)data.size());
        process_buffer(iter->second);
    }

    //  Parse and dispatch the next request on `connection`, if one is complete.
    void process_buffer(const std::shared_ptr<HttpConnection>& connection){
        QTcpSocket* socket = connection->socket;
        if (socket == nullptr || connection->busy){
            return;
        }

        HttpRequest request;
        HttpParseResult result = parse_http_request(connection->buffer, request);
        switch (result.status){
        case HttpParseStatus::NEED_MORE:
            if (result.expect_continue && !connection->sent_continue){
                connection->sent_continue = true;
                socket->write("HTTP/1.1 100 Continue\r\n\r\n");
            }
            return;
        case HttpParseStatus::ERROR:{
            logger.log("[AgentServer] Rejected HTTP request: " + result.error_message, COLOR_RED);
            std::string bytes = serialize_http_response(
                HttpResponse::text(result.error_status, result.error_message + "\n"), false
            );
            socket->write(bytes.data(), (qint64)bytes.size());
            socket->disconnectFromHost();
            return;
        }
        case HttpParseStatus::COMPLETE:
            break;
        }

        connection->busy = true;
        connection->sent_continue = false;
        request.peer_address = socket->peerAddress().toString().toStdString();

        {
            std::lock_guard<std::mutex> lg(inflight_lock);
            inflight++;
        }
        std::weak_ptr<HttpConnection> weak = connection;
        std::thread([this, weak, request = std::move(request)]{
            run_handler(weak, request);
        }).detach();
    }

    //  ---- Worker thread. ----

    void run_handler(std::weak_ptr<HttpConnection> weak, const HttpRequest& request){
        HttpResponse response;
        try{
            response = handler(request);
        }catch (const std::exception& e){
            logger.log(std::string("[AgentServer] Request handler failed: ") + e.what(), COLOR_RED);
            response = HttpResponse::text(500, "Internal server error.\n");
        }catch (...){
            logger.log("[AgentServer] Request handler failed.", COLOR_RED);
            response = HttpResponse::text(500, "Internal server error.\n");
        }
        bool keep_alive = request.keep_alive;
        std::string bytes = serialize_http_response(response, keep_alive);

        {
            std::lock_guard<std::mutex> lg(post_lock);
            if (post_target != nullptr){
                QMetaObject::invokeMethod(post_target, [this, weak, bytes = std::move(bytes), keep_alive]{
                    send_response(weak, bytes, keep_alive);
                }, Qt::QueuedConnection);
            }
        }

        {
            std::lock_guard<std::mutex> lg(inflight_lock);
            inflight--;
        }
        inflight_cv.notify_all();
    }

    //  ---- Back on `thread`. ----

    void send_response(const std::weak_ptr<HttpConnection>& weak, const std::string& bytes, bool keep_alive){
        std::shared_ptr<HttpConnection> connection = weak.lock();
        if (!connection || connection->socket == nullptr){
            return;     //  client went away while the handler ran
        }
        QTcpSocket* socket = connection->socket;
        socket->write(bytes.data(), (qint64)bytes.size());
        connection->busy = false;
        if (!keep_alive){
            socket->disconnectFromHost();
            return;
        }
        //  The client may have sent its next request already.
        process_buffer(connection);
    }
};



HttpServer::HttpServer(Logger& logger, HttpHandler handler)
    : m_internal(std::make_unique<Internal>(logger, std::move(handler)))
{}
HttpServer::~HttpServer(){
    stop();
}

std::string HttpServer::start(uint16_t port, bool bind_all_interfaces){
    Internal& data = *m_internal;
    if (data.thread.isRunning()){
        return "The server is already running.";
    }

    data.thread.setObjectName("AgentServer HTTP");
    data.thread.start();
    data.context = new QObject();
    data.context->moveToThread(&data.thread);

    std::string error;
    bool dispatched = QMetaObject::invokeMethod(data.context, [&]{
        data.server = new QTcpServer(data.context);
        QObject::connect(data.server, &QTcpServer::newConnection, data.context, [&data]{
            data.on_new_connection();
        });
        QHostAddress address = bind_all_interfaces ? QHostAddress(QHostAddress::Any) : QHostAddress(QHostAddress::LocalHost);
        if (!data.server->listen(address, port)){
            error = "Unable to listen on port " + std::to_string(port) + ": " +
                data.server->errorString().toStdString();
            return;
        }
        data.port.store(data.server->serverPort(), std::memory_order_release);
    }, Qt::BlockingQueuedConnection);
    if (!dispatched){
        //  E.g. no QCoreApplication exists, so the server thread has no event loop.
        error = "Unable to start the server thread.";
    }

    if (!error.empty()){
        data.logger.log("[AgentServer] " + error, COLOR_RED);
        stop();
        return error;
    }

    {
        std::lock_guard<std::mutex> lg(data.post_lock);
        data.post_target = data.context;
    }
    data.listening.store(true, std::memory_order_release);
    data.logger.log(
        "[AgentServer] Listening on " + std::string(bind_all_interfaces ? "all interfaces" : "127.0.0.1") +
        ", port " + std::to_string(data.port.load()),
        COLOR_BLUE
    );
    return "";
}

void HttpServer::stop(){
    Internal& data = *m_internal;
    if (!data.thread.isRunning()){
        return;
    }
    data.listening.store(false, std::memory_order_release);

    //  1. On the server thread: stop accepting, then drop and delete every
    //     connection and the listener. (Socket objects must be deleted on the
    //     thread that owns them.)
    QMetaObject::invokeMethod(data.context, [&data]{
        for (auto& item : data.connections){
            QTcpSocket* socket = item.second->socket;
            if (socket != nullptr){
                QObject::disconnect(socket, nullptr, data.context, nullptr);
                socket->abort();
                delete socket;
            }
        }
        data.connections.clear();
        delete data.server;     //  also deletes any not-yet-accepted connections
        data.server = nullptr;
    }, Qt::BlockingQueuedConnection);

    //  2. Stop workers from posting responses, then wait for running handlers.
    {
        std::lock_guard<std::mutex> lg(data.post_lock);
        data.post_target = nullptr;
    }
    {
        std::unique_lock<std::mutex> lg(data.inflight_lock);
        while (!data.inflight_cv.wait_for(lg, std::chrono::seconds(5), [&]{ return data.inflight == 0; })){
            data.logger.log(
                "[AgentServer] Waiting for " + std::to_string(data.inflight) + " request(s) to finish...",
                COLOR_ORANGE
            );
        }
    }

    //  3. Stop the thread. `context` is a plain QObject with nothing left on it,
    //     so it can be deleted here once its thread has finished.
    data.thread.quit();
    data.thread.wait();
    delete data.context;
    data.context = nullptr;
    data.port.store(0, std::memory_order_release);
    data.logger.log("[AgentServer] Stopped.", COLOR_BLUE);
}

bool HttpServer::is_listening() const{
    return m_internal->listening.load(std::memory_order_acquire);
}
uint16_t HttpServer::port() const{
    return m_internal->port.load(std::memory_order_acquire);
}



}
}
