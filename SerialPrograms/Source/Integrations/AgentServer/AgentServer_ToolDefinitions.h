/*  Agent Server: Tool Definitions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Loads the shared MCP interface, AgentTools.json, which both this app and the
 *  Python package (pokemon_automation.mcp_server) serve, so AI agents see the same
 *  tools whichever server they connect to. The file is compiled into the app (see
 *  `agent_tools_json_text()`).
 *
 *  Also validates tool arguments against the tools' JSON Schemas. Only the schema
 *  keywords AgentTools.json uses are supported: type, properties, required,
 *  additionalProperties, items, minItems, maxItems, minimum, maximum, enum, anyOf.
 */

#ifndef PokemonAutomation_AgentServer_ToolDefinitions_H
#define PokemonAutomation_AgentServer_ToolDefinitions_H

#include <map>
#include <string>
#include <vector>
#include "3rdParty-Core/nlohmann/json.hpp"

namespace PokemonAutomation{
namespace AgentServer{


//  The contents of AgentTools.json, embedded at build time.
const std::string& agent_tools_json_text();


struct AgentToolDefinition{
    std::string name;
    std::string description;
    nlohmann::json input_schema;    //  self-contained: all "$ref"s are inlined
};


class AgentToolDefinitions{
public:
    //  Parse the definition file and keep the tools implemented by `host`
    //  ("app" or "python"). Throws std::runtime_error if the JSON is malformed or a
    //  "$ref" names an unknown definition.
    AgentToolDefinitions(const std::string& json_text, const std::string& host);

    const std::string& server_name() const{ return m_server_name; }
    const std::string& instructions() const{ return m_instructions; }
    const std::vector<AgentToolDefinition>& tools() const{ return m_tools; }

    //  Returns nullptr if there is no tool named `name`.
    const AgentToolDefinition* find(const std::string& name) const;

    //  The `tools` array for an MCP tools/list response.
    nlohmann::json tools_list() const;

    //  Check `arguments` against the tool's input schema.
    //  Returns an empty string if valid, otherwise a message for the agent such as
    //  "steps[0].hold_ms must be >= 1".
    std::string validate_arguments(const AgentToolDefinition& tool, const nlohmann::json& arguments) const;

    //  Return `arguments` with every missing top-level property that has a schema
    //  "default" filled in. (Defaults of nested objects, such as run_inputs steps,
    //  are applied by the code that parses them.)
    static nlohmann::json with_defaults(const AgentToolDefinition& tool, const nlohmann::json& arguments);

private:
    std::string m_server_name;
    std::string m_instructions;
    std::vector<AgentToolDefinition> m_tools;
    std::map<std::string, size_t> m_index;
};


//  Replace every {"$ref": "#/definitions/<name>", ...siblings} in `schema` with a copy
//  of that definition merged with the siblings (siblings win). Same behavior as
//  `resolve_refs()` in pokemon_automation/agent_tools.py.
//  Throws std::runtime_error for an unknown or unsupported reference.
nlohmann::json resolve_schema_refs(const nlohmann::json& schema, const nlohmann::json& definitions);

//  Validate `value` against `schema`. Returns an empty string if valid, otherwise a
//  message that starts with `path` (the location of `value`, e.g. "steps[0]").
std::string validate_json_schema(const nlohmann::json& schema, const nlohmann::json& value, const std::string& path);



}
}
#endif
