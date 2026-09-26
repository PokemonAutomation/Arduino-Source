/*  Agent Server: HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A minimal HTTP/1.1 server on Qt's QTcpServer, just enough to serve MCP's
 *  "Streamable HTTP" transport to AI agents (see AgentServer_McpServer.h).
 *
 *  Qt's own QHttpServer module is not used because it is GPL-3.0-only, and this
 *  project does not take plain GPL dependencies. QTcpServer (Qt Network) is LGPL.
 *
 *  Supported: one request at a time per connection, keep-alive, Content-Length
 *  request bodies, "Expect: 100-continue". Not supported (answered with an error):
 *  chunked request bodies, pipelining, upgrades.
 */

#ifndef PokemonAutomation_AgentServer_HttpServer_H
#define PokemonAutomation_AgentServer_HttpServer_H

#include <stdint.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace PokemonAutomation{
    class Logger;
namespace AgentServer{


struct HttpRequest{
    std::string method;         //  e.g. "POST"
    std::string path;           //  e.g. "/mcp" (without the query string)
    std::string query;          //  the part after '?', if any
    std::map<std::string, std::string> headers;    //  keys are lower-case
    std::string body;
    std::string peer_address;   //  remote IP address, for logging
    bool keep_alive = true;     //  HTTP/1.1 default, unless "Connection: close"

    //  Returns the header value, or an empty string if absent. `name` must be lower-case.
    const std::string& header(const std::string& name) const;
};

struct HttpResponse{
    int status = 200;
    std::string content_type;   //  empty = no Content-Type header (e.g. for 202)
    std::string body;
    std::vector<std::pair<std::string, std::string>> headers;   //  extra headers

    static HttpResponse text(int status, std::string body);
    static HttpResponse json(int status, std::string body);
};

//  Called on a worker thread for every complete request. May block (e.g. while an
//  agent's tool call runs); other connections are served meanwhile.
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;


//  Result of trying to parse one request from the front of a connection's buffer.
enum class HttpParseStatus{
    NEED_MORE,      //  incomplete; wait for more bytes
    COMPLETE,       //  `request` is filled and its bytes were removed from the buffer
    ERROR,          //  malformed or unsupported; `error_status` says which HTTP error
};
struct HttpParseResult{
    HttpParseStatus status = HttpParseStatus::NEED_MORE;
    int error_status = 0;       //  400, 411, 413, 431, 501 or 505 when ERROR
    std::string error_message;
    bool expect_continue = false;   //  headers ask for "100 Continue" before the body
};

//  Parse one HTTP/1.1 request from the front of `buffer`.
//  Limits: 32 KB of headers, `max_body_bytes` of body.
//  This is separate from the server so it can be unit-tested.
HttpParseResult parse_http_request(
    std::string& buffer, HttpRequest& request,
    size_t max_body_bytes = 8 * 1024 * 1024
);

//  Serialize a response. `keep_alive` selects the Connection header.
std::string serialize_http_response(const HttpResponse& response, bool keep_alive);



//  Listens on one address/port and hands each request to `handler`.
//
//  Threads: the QTcpServer and all sockets live on a private thread with its own
//  Qt event loop, so this works whether or not the caller runs one. (A
//  QCoreApplication must exist, as it always does in the app.) Each request
//  runs on its own worker thread. `stop()` (and the destructor) closes the listener
//  and all connections and waits for running handlers to return.
class HttpServer{
    HttpServer(const HttpServer&) = delete;
    void operator=(const HttpServer&) = delete;

public:
    HttpServer(Logger& logger, HttpHandler handler);
    ~HttpServer();

    //  Start listening. `bind_all_interfaces` = false binds 127.0.0.1 only.
    //  Returns an empty string on success, or the error (e.g. port in use).
    std::string start(uint16_t port, bool bind_all_interfaces);

    void stop();

    bool is_listening() const;
    uint16_t port() const;      //  the actual port (useful when started with port 0)

private:
    struct Internal;
    std::unique_ptr<Internal> m_internal;
};



}
}
#endif
