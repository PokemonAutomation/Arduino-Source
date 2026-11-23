/*  Abstract HTTP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The interface of a minimal HTTP/1.1 server, for a single local client
 *  that issues one request at a time (for example, an MCP client talking
 *  to the AI Agent Server). Implementations own the transport; this class
 *  owns the request/response types, the handler contract, and the
 *  lifecycle events.
 *
 *  Design notes:
 *  - One connection at a time: while a client is connected, additional
 *    clients are rejected ("on_connect_rejected").
 *  - Requests are processed strictly in order. Bytes of a pipelined
 *    request simply wait in the buffer until the previous response has
 *    been sent.
 *  - The "Handler" may block for a long time; it runs on a thread from
 *    the implementation's thread pool. Poll
 *    "HandlerContext::connection_alive()" between long-running steps to
 *    bail out early. Once it returns false, the response is discarded
 *    even if you set one.
 *  - No TLS. Request bodies: "Content-Length" only. Response bodies:
 *    always "Content-Length" (no chunked). No upgrades.
 *
 *  Threading:
 *  - All "Listener" callbacks run on threads from the implementation's
 *    thread pool, never on the calling thread.
 *  - Do not call "start()" or "stop()" from within a "Listener"
 *    callback.
 *  - "start()" and "stop()" are not thread-safe with each other: do not
 *    call them concurrently from different threads.
 *  - Implementations keep an accept thread and a per-connection thread
 *    alive for as long as they run, so the thread pool they use should be
 *    an unlimited, long-lived one (for example
 *    "GlobalThreadPools::unlimited_normal()").
 *
 */

#ifndef PokemonAutomation_AbstractHttpServer_H
#define PokemonAutomation_AbstractHttpServer_H

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Common/Cpp/ListenerSet.h"

namespace PokemonAutomation{



class AbstractHttpServer{
public:
    struct Request{
        std::string method;                    //  e.g. "POST"
        std::string path;                      //  e.g. "/mcp" (without the query)
        std::string query;                     //  the part after '?', if any
        std::map<std::string, std::string> headers;   //  names are lower-cased
        std::string body;

        //  "name" is lower-cased first; returns "" if the header is absent.
        const char* header(const std::string& name) const{
            auto lower = [](std::string value){
                std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
                    return (char)std::tolower(c);
                });
                return value;
            };
            const auto it = headers.find(lower(name));
            return (it == headers.end()) ? "" : it->second.c_str();
        }
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
        virtual ~HandlerContext() = default;

        //  False once the connection this request came in on is closed or
        //  the server is stopping. Once false, the response is discarded
        //  even if you set one.
        virtual bool connection_alive() const = 0;
    };

    //  Runs on a thread from the implementation's thread pool, never on
    //  the transport's connection thread. It may block (for example,
    //  while a tool runs on the program thread), but it must eventually
    //  return: "stop()" blocks until the in-flight handler returns. It
    //  must not throw; exceptions are turned into a 500 response.
    using Handler = std::function<void(const Request& request, Response& response, HandlerContext& context)>;

    struct Listener{
        //  Called after "start()" finishes. If successful, "error_message"
        //  is empty. Otherwise it contains the error message and the
        //  server is not listening; call "stop()" to clean up before
        //  retrying.
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
    virtual ~AbstractHttpServer() = default;

    //  Starts listening. Asynchronous: "Listener::on_listen_finished" is
    //  called on a thread from the thread pool with the result.
    //  "handler" receives every fully parsed request; if it is null, all
    //  requests get a 500. Returns without starting if the server is
    //  already started.
    virtual void start(const std::string& address, uint16_t port, Handler handler) = 0;

    //  Stops the server and blocks until it is fully down: the active
    //  connection is closed, the in-flight handler (if any) is waited
    //  for, and the transport's worker threads are joined. Afterwards
    //  "start()" can be called again.
    virtual void stop() = 0;

    //  True if "start()" has been called and the server has not been
    //  stopped again.
    virtual bool is_running() const = 0;


protected:
    ListenerSet<Listener> m_listeners;
};



}
#endif
