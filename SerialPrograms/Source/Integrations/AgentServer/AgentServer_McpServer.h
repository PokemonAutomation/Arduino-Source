/*  Agent Server: MCP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The Model Context Protocol (MCP) over "Streamable HTTP", served at POST /mcp.
 *  Any MCP client (Claude Code, Claude Desktop, Codex, ...) can connect to it and
 *  call the tools in AgentTools.json. This class does the protocol; the tools
 *  themselves are implemented by a `McpToolHandler` (the "AI Agent Server" program).
 *
 *  Protocol support:
 *  - The initialize handshake, protocol versions 2024-11-05 through 2025-11-25.
 *    Newer clients first probe `server/discover` (protocol 2026-07-28); that gets
 *    "method not found", which tells them to fall back to the handshake.
 *  - Requests: initialize, ping, tools/list, tools/call. Notifications are accepted
 *    and ignored. Responses are plain JSON (no server-sent-event streams).
 *  - GET /mcp (a server-to-client event stream) is not offered: 405.
 *  - DELETE /mcp ends the session.
 *
 *  Security (the server can press any button on the user's console):
 *  - Bearer token: when `McpServerConfig::access_token` is set, every request needs
 *    "Authorization: Bearer <token>".
 *  - DNS-rebinding protection: requests from web pages (with an Origin header) are
 *    rejected unless the origin is this machine, and in localhost-only mode the Host
 *    header must name this machine.
 */

#ifndef PokemonAutomation_AgentServer_McpServer_H
#define PokemonAutomation_AgentServer_McpServer_H

#include <stdint.h>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "3rdParty-Core/nlohmann/json.hpp"
#include "AgentServer_HttpServer.h"
#include "AgentServer_ToolDefinitions.h"

namespace PokemonAutomation{
    class Logger;
namespace AgentServer{


//  One content block of a tool result.
struct McpContent{
    enum class Type{ TEXT, IMAGE };
    Type type = Type::TEXT;
    std::string text;               //  TEXT
    std::string base64_data;        //  IMAGE
    std::string mime_type;          //  IMAGE, e.g. "image/jpeg"

    static McpContent make_text(std::string text);
    static McpContent make_image(std::string base64_data, std::string mime_type);
};

//  What a tool returns. `is_error` marks a failure the agent should see and react
//  to (bad arguments, the user has taken control, no video, ...).
struct McpToolResult{
    std::vector<McpContent> content;
    bool is_error = false;

    static McpToolResult text(std::string message);
    static McpToolResult error(std::string message);
};


//  Implements the tools. `call_tool()` runs on an HTTP worker thread and may block
//  while inputs execute. It is only called for tools in the definitions, with
//  arguments that passed schema validation and have top-level defaults filled in.
//  Exceptions are caught and reported to the agent as tool errors.
class McpToolHandler{
public:
    virtual ~McpToolHandler() = default;
    virtual McpToolResult call_tool(const std::string& name, const nlohmann::json& arguments) = 0;
};


struct McpServerConfig{
    std::string server_version;     //  reported to clients, e.g. the app version
    std::string access_token;       //  empty = no authentication
    bool localhost_only = true;     //  check the Host header (see above)
};


class McpServer{
public:
    McpServer(
        Logger& logger,
        const AgentToolDefinitions& definitions,
        McpToolHandler& handler,
        McpServerConfig config
    );

    //  Handle one HTTP request. Thread-safe; use as the `HttpServer` handler.
    HttpResponse handle(const HttpRequest& request);

    //  Protocol versions accepted in initialize, newest first.
    static const std::vector<std::string>& supported_protocol_versions();

    size_t active_sessions() const;

private:
    //  Returns a response if the request is not allowed, else nothing.
    std::optional<HttpResponse> check_access(const HttpRequest& request) const;

    //  Handle one JSON-RPC message. Returns the response, or null for a
    //  notification or response (which get no reply).
    nlohmann::json handle_message(const nlohmann::json& message, std::string& new_session_id);

    nlohmann::json handle_initialize(const nlohmann::json& params, std::string& new_session_id);
    nlohmann::json handle_tools_call(const nlohmann::json& params);

private:
    Logger& m_logger;
    const AgentToolDefinitions& m_definitions;
    McpToolHandler& m_handler;
    const McpServerConfig m_config;

    mutable std::mutex m_lock;
    std::map<std::string, uint64_t> m_sessions;     //  session ID -> creation counter
    uint64_t m_session_counter = 0;
};



}
}
#endif
