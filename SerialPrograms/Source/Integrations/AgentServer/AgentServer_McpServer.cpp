/*  Agent Server: MCP Server
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cctype>
#include <chrono>
#include <random>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "AgentServer_McpServer.h"

namespace PokemonAutomation{
namespace AgentServer{

using nlohmann::json;



McpContent McpContent::make_text(std::string text){
    McpContent ret;
    ret.type = Type::TEXT;
    ret.text = std::move(text);
    return ret;
}
McpContent McpContent::make_image(std::string base64_data, std::string mime_type){
    McpContent ret;
    ret.type = Type::IMAGE;
    ret.base64_data = std::move(base64_data);
    ret.mime_type = std::move(mime_type);
    return ret;
}
McpToolResult McpToolResult::text(std::string message){
    McpToolResult ret;
    ret.content.emplace_back(McpContent::make_text(std::move(message)));
    return ret;
}
McpToolResult McpToolResult::error(std::string message){
    McpToolResult ret = text(std::move(message));
    ret.is_error = true;
    return ret;
}



namespace{

//  JSON-RPC 2.0 error codes.
const int PARSE_ERROR       = -32700;
const int INVALID_REQUEST   = -32600;
const int METHOD_NOT_FOUND  = -32601;
const int INVALID_PARAMS    = -32602;

json rpc_error(const json& id, int code, const std::string& message){
    return {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {{"code", code}, {"message", message}}},
    };
}
json rpc_result(const json& id, json result){
    return {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", std::move(result)},
    };
}

HttpResponse json_response(int status, const json& body){
    return HttpResponse::json(status, body.dump());
}

std::string to_lower(std::string text){
    for (char& ch : text){
        ch = (char)std::tolower((unsigned char)ch);
    }
    return text;
}

//  "localhost:8765" -> "localhost", "[::1]:8765" -> "[::1]", "127.0.0.1" -> "127.0.0.1"
std::string host_without_port(const std::string& host){
    if (host.starts_with("[")){
        size_t end = host.find(']');
        return end == std::string::npos ? host : host.substr(0, end + 1);
    }
    size_t colon = host.find(':');
    return colon == std::string::npos ? host : host.substr(0, colon);
}
bool is_local_host_name(const std::string& host){
    std::string name = to_lower(host_without_port(host));
    return name == "localhost" || name == "127.0.0.1" || name == "[::1]";
}

//  Compare without an early exit, so response timing doesn't reveal the token.
bool constant_time_equals(const std::string& a, const std::string& b){
    if (a.size() != b.size()){
        return false;
    }
    unsigned char diff = 0;
    for (size_t c = 0; c < a.size(); c++){
        diff |= (unsigned char)(a[c] ^ b[c]);
    }
    return diff == 0;
}

std::string random_hex(size_t bytes){
    static std::mutex lock;
    static std::mt19937_64 rng(std::random_device{}());
    std::lock_guard<std::mutex> lg(lock);
    const char* digits = "0123456789abcdef";
    std::string ret;
    for (size_t c = 0; c < bytes; c++){
        uint8_t byte = (uint8_t)(rng() & 0xff);
        ret += digits[byte >> 4];
        ret += digits[byte & 15];
    }
    return ret;
}

json to_json(const McpToolResult& result){
    json content = json::array();
    for (const McpContent& block : result.content){
        if (block.type == McpContent::Type::IMAGE){
            content.push_back({
                {"type", "image"},
                {"data", block.base64_data},
                {"mimeType", block.mime_type},
            });
        }else{
            content.push_back({
                {"type", "text"},
                {"text", block.text},
            });
        }
    }
    return {
        {"content", std::move(content)},
        {"isError", result.is_error},
    };
}

const size_t MAX_SESSIONS = 64;

}



const std::vector<std::string>& McpServer::supported_protocol_versions(){
    static const std::vector<std::string> versions{
        "2025-11-25",
        "2025-06-18",
        "2025-03-26",
        "2024-11-05",
    };
    return versions;
}


McpServer::McpServer(
    Logger& logger,
    const AgentToolDefinitions& definitions,
    McpToolHandler& handler,
    McpServerConfig config
)
    : m_logger(logger)
    , m_definitions(definitions)
    , m_handler(handler)
    , m_config(std::move(config))
{}

size_t McpServer::active_sessions() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_sessions.size();
}


std::optional<HttpResponse> McpServer::check_access(const HttpRequest& request) const{
    //  A web page can make the browser send requests here. Browsers always send
    //  Origin with those; MCP clients generally don't send one at all.
    const std::string& origin = request.header("origin");
    if (!origin.empty()){
        size_t scheme_end = origin.find("://");
        std::string host = scheme_end == std::string::npos ? "" : origin.substr(scheme_end + 3);
        if (!is_local_host_name(host)){
            m_logger.log("[AgentServer] Rejected request from web origin: " + origin, COLOR_RED);
            return HttpResponse::text(403, "Requests from web pages are not allowed.\n");
        }
    }

    //  DNS rebinding: a remote site's domain resolving to 127.0.0.1 still carries
    //  that domain in the Host header.
    const std::string& host = request.header("host");
    if (m_config.localhost_only && !host.empty() && !is_local_host_name(host)){
        m_logger.log("[AgentServer] Rejected request for host: " + host, COLOR_RED);
        return HttpResponse::text(403, "Invalid Host header.\n");
    }

    if (!m_config.access_token.empty()){
        const std::string& authorization = request.header("authorization");
        if (!constant_time_equals(authorization, "Bearer " + m_config.access_token)){
            m_logger.log("[AgentServer] Rejected request without a valid access token from " + request.peer_address, COLOR_RED);
            return json_response(401, rpc_error(nullptr, INVALID_REQUEST,
                "Missing or wrong access token. Send \"Authorization: Bearer <token>\" "
                "with the token shown in SerialPrograms' AI Agent Server program."
            ));
        }
    }
    return std::nullopt;
}


HttpResponse McpServer::handle(const HttpRequest& request){
    if (request.path != "/mcp" && request.path != "/mcp/"){
        return HttpResponse::text(404, "Not found. The MCP endpoint is /mcp.\n");
    }
    if (std::optional<HttpResponse> denied = check_access(request)){
        return *denied;
    }

    const std::string& session_id = request.header("mcp-session-id");

    if (request.method == "DELETE"){
        std::lock_guard<std::mutex> lg(m_lock);
        if (session_id.empty() || m_sessions.erase(session_id) == 0){
            return HttpResponse::text(404, "Session not found.\n");
        }
        return HttpResponse::text(200, "Session ended.\n");
    }
    if (request.method != "POST"){
        //  GET would open a server-to-client event stream, which this server doesn't offer.
        HttpResponse response = HttpResponse::text(405, "Use POST.\n");
        response.headers.emplace_back("Allow", "POST, DELETE");
        return response;
    }

    std::string content_type = to_lower(request.header("content-type"));
    if (!content_type.empty() && !content_type.starts_with("application/json")){
        return HttpResponse::text(415, "Content-Type must be application/json.\n");
    }

    if (!session_id.empty()){
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_sessions.find(session_id) == m_sessions.end()){
            //  The spec's signal for "session expired; initialize again".
            return json_response(404, rpc_error(nullptr, INVALID_REQUEST, "Session not found."));
        }
    }

    json body;
    try{
        body = json::parse(request.body);
    }catch (const json::parse_error&){
        return json_response(400, rpc_error(nullptr, PARSE_ERROR, "Parse error: the body is not valid JSON."));
    }

    std::string new_session_id;
    json reply;
    if (body.is_array()){
        //  JSON-RPC batch (protocol 2025-03-26).
        if (body.empty()){
            return json_response(400, rpc_error(nullptr, INVALID_REQUEST, "Empty batch."));
        }
        reply = json::array();
        for (const json& message : body){
            json response = handle_message(message, new_session_id);
            if (!response.is_null()){
                reply.push_back(std::move(response));
            }
        }
        if (reply.empty()){
            reply = nullptr;
        }
    }else{
        reply = handle_message(body, new_session_id);
    }

    HttpResponse response;
    if (reply.is_null()){
        response.status = 202;      //  only notifications/responses: nothing to return
    }else{
        response = json_response(200, reply);
    }
    if (!new_session_id.empty()){
        response.headers.emplace_back("Mcp-Session-Id", new_session_id);
    }
    return response;
}


json McpServer::handle_message(const json& message, std::string& new_session_id){
    if (!message.is_object() || message.value("jsonrpc", "") != "2.0"){
        return rpc_error(nullptr, INVALID_REQUEST, "Not a JSON-RPC 2.0 message.");
    }
    auto method_iter = message.find("method");
    if (method_iter == message.end()){
        return nullptr;     //  a response to a server request; this server sends none
    }
    if (!method_iter->is_string()){
        return rpc_error(message.value("id", json()), INVALID_REQUEST, "\"method\" must be a string.");
    }
    const std::string method = method_iter->get<std::string>();

    if (!message.contains("id")){
        //  Notification, e.g. notifications/initialized or notifications/cancelled.
        return nullptr;
    }
    const json& id = message["id"];
    json params = message.value("params", json::object());

    try{
        if (method == "initialize"){
            return rpc_result(id, handle_initialize(params, new_session_id));
        }
        if (method == "ping"){
            return rpc_result(id, json::object());
        }
        if (method == "tools/list"){
            return rpc_result(id, {{"tools", m_definitions.tools_list()}});
        }
        if (method == "tools/call"){
            if (!params.is_object() || !params.contains("name") || !params["name"].is_string()){
                return rpc_error(id, INVALID_PARAMS, "tools/call needs a \"name\".");
            }
            std::string name = params["name"].get<std::string>();
            if (m_definitions.find(name) == nullptr){
                return rpc_error(id, INVALID_PARAMS, "Unknown tool: " + name);
            }
            return rpc_result(id, handle_tools_call(params));
        }
    }catch (const std::exception& e){
        m_logger.log(std::string("[AgentServer] Error handling ") + method + ": " + e.what(), COLOR_RED);
        return rpc_error(id, -32603, std::string("Internal error: ") + e.what());
    }

    //  Includes "server/discover" (protocol 2026-07-28): "method not found" tells
    //  newer clients to fall back to the initialize handshake.
    return rpc_error(id, METHOD_NOT_FOUND, "Method not found: " + method);
}


json McpServer::handle_initialize(const json& params, std::string& new_session_id){
    std::string requested = params.is_object() ? params.value("protocolVersion", "") : "";
    const std::vector<std::string>& supported = supported_protocol_versions();
    std::string version = supported.front();
    for (const std::string& v : supported){
        if (v == requested){
            version = v;
        }
    }

    std::string client = "unknown client";
    if (params.is_object() && params.contains("clientInfo") && params["clientInfo"].is_object()){
        const json& info = params["clientInfo"];
        client = info.value("name", "unknown client") + " " + info.value("version", "");
    }

    new_session_id = random_hex(16);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_sessions[new_session_id] = ++m_session_counter;
        //  Clients that vanish never send DELETE; drop the oldest sessions.
        while (m_sessions.size() > MAX_SESSIONS){
            auto oldest = m_sessions.begin();
            for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter){
                if (iter->second < oldest->second){
                    oldest = iter;
                }
            }
            m_sessions.erase(oldest);
        }
    }
    m_logger.log("[AgentServer] Agent connected: " + client + " (protocol " + version + ")", COLOR_BLUE);

    return {
        {"protocolVersion", version},
        {"capabilities", {{"tools", {{"listChanged", false}}}}},
        {"serverInfo", {
            {"name", m_definitions.server_name()},
            {"version", m_config.server_version},
        }},
        {"instructions", m_definitions.instructions()},
    };
}


json McpServer::handle_tools_call(const json& params){
    const AgentToolDefinition& tool = *m_definitions.find(params["name"].get<std::string>());
    json arguments = params.value("arguments", json::object());
    if (arguments.is_null()){
        arguments = json::object();
    }

    std::string error = m_definitions.validate_arguments(tool, arguments);
    if (!error.empty()){
        //  A tool error (not a protocol error), so the agent can read it and retry.
        return to_json(McpToolResult::error("Invalid arguments for " + tool.name + ": " + error));
    }
    arguments = AgentToolDefinitions::with_defaults(tool, arguments);

    auto start = std::chrono::steady_clock::now();
    McpToolResult result;
    try{
        result = m_handler.call_tool(tool.name, arguments);
    }catch (const std::exception& e){
        result = McpToolResult::error(e.what());
    }
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    m_logger.log(
        "[AgentServer] " + tool.name + (result.is_error ? " failed" : " done") + " (" + std::to_string(millis) + " ms)",
        result.is_error ? COLOR_RED : COLOR_DARKGREEN
    );
    return to_json(result);
}



}
}
