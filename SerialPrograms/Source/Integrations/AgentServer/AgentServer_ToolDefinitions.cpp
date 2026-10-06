/*  Agent Server: Tool Definitions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <sstream>
#include <stdexcept>
#include "AgentServer_ToolDefinitions.h"

namespace PokemonAutomation{
namespace AgentServer{

using nlohmann::json;



json resolve_schema_refs(const json& schema, const json& definitions){
    if (schema.is_array()){
        json ret = json::array();
        for (const json& item : schema){
            ret.push_back(resolve_schema_refs(item, definitions));
        }
        return ret;
    }
    if (!schema.is_object()){
        return schema;
    }
    auto ref = schema.find("$ref");
    if (ref != schema.end()){
        const std::string prefix = "#/definitions/";
        std::string target = ref->is_string() ? ref->get<std::string>() : "";
        if (!target.starts_with(prefix)){
            throw std::runtime_error("Unsupported $ref: " + target);
        }
        std::string name = target.substr(prefix.size());
        auto definition = definitions.find(name);
        if (definition == definitions.end()){
            throw std::runtime_error("Unknown $ref: " + target);
        }
        json merged = *definition;
        for (auto iter = schema.begin(); iter != schema.end(); ++iter){
            if (iter.key() != "$ref"){
                merged[iter.key()] = iter.value();
            }
        }
        return resolve_schema_refs(merged, definitions);
    }
    json ret = json::object();
    for (auto iter = schema.begin(); iter != schema.end(); ++iter){
        ret[iter.key()] = resolve_schema_refs(iter.value(), definitions);
    }
    return ret;
}



namespace{

std::string json_type_name(const json& value){
    if (value.is_null()) return "null";
    if (value.is_boolean()) return "boolean";
    if (value.is_number_integer()) return "integer";
    if (value.is_number()) return "number";
    if (value.is_string()) return "string";
    if (value.is_array()) return "array";
    return "object";
}

bool matches_type(const std::string& type, const json& value){
    if (type == "object") return value.is_object();
    if (type == "array") return value.is_array();
    if (type == "string") return value.is_string();
    if (type == "boolean") return value.is_boolean();
    if (type == "null") return value.is_null();
    if (type == "number") return value.is_number();
    if (type == "integer"){
        if (value.is_number_integer()){
            return true;
        }
        //  JSON has one number type; 5.0 is an integer as far as schemas go.
        if (value.is_number_float()){
            double x = value.get<double>();
            return std::isfinite(x) && x == std::floor(x);
        }
        return false;
    }
    return false;
}

std::string number_text(double x){
    std::ostringstream ss;
    ss << x;
    return ss.str();
}

//  "a string", "an integer", "a string or array"
std::string with_article(const std::string& type_list){
    char first = type_list.empty() ? 'x' : type_list[0];
    bool vowel = first == 'a' || first == 'e' || first == 'i' || first == 'o' || first == 'u';
    return (vowel ? "an " : "a ") + type_list;
}

std::string where(const std::string& path){
    return path.empty() ? "arguments" : path;
}

}


std::string validate_json_schema(const json& schema, const json& value, const std::string& path){
    if (!schema.is_object()){
        return "";
    }

    auto any_of = schema.find("anyOf");
    if (any_of != schema.end() && any_of->is_array()){
        //  Valid if any option accepts the value. Otherwise report the error of the
        //  option with the value's type (e.g. "must be <= 1" for a stick array), or
        //  list the accepted types if none has it.
        std::string type_matched_error;
        std::string expected;
        bool valid = false;
        for (const json& option : *any_of){
            std::string error = validate_json_schema(option, value, path);
            if (error.empty()){
                valid = true;
                break;
            }
            auto option_type = option.find("type");
            if (option_type != option.end() && option_type->is_string()){
                expected += (expected.empty() ? "" : " or ") + option_type->get<std::string>();
                if (type_matched_error.empty() && matches_type(option_type->get<std::string>(), value)){
                    type_matched_error = error;
                }
            }
        }
        if (!valid){
            return type_matched_error.empty()
                ? where(path) + " must be " + with_article(expected) + ", not " + json_type_name(value)
                : type_matched_error;
        }
    }

    auto type = schema.find("type");
    if (type != schema.end() && type->is_string() && !matches_type(type->get<std::string>(), value)){
        return where(path) + " must be " + with_article(type->get<std::string>()) + ", not " + json_type_name(value);
    }

    auto enum_values = schema.find("enum");
    if (enum_values != schema.end() && enum_values->is_array()){
        bool found = false;
        for (const json& allowed : *enum_values){
            found |= allowed == value;
        }
        if (!found){
            return where(path) + " must be one of " + enum_values->dump();
        }
    }

    if (value.is_number()){
        double x = value.get<double>();
        auto minimum = schema.find("minimum");
        if (minimum != schema.end() && x < minimum->get<double>()){
            return where(path) + " must be >= " + number_text(minimum->get<double>());
        }
        auto maximum = schema.find("maximum");
        if (maximum != schema.end() && x > maximum->get<double>()){
            return where(path) + " must be <= " + number_text(maximum->get<double>());
        }
    }

    if (value.is_array()){
        auto min_items = schema.find("minItems");
        if (min_items != schema.end() && value.size() < min_items->get<size_t>()){
            return where(path) + " must have at least " + std::to_string(min_items->get<size_t>()) + " item(s)";
        }
        auto max_items = schema.find("maxItems");
        if (max_items != schema.end() && value.size() > max_items->get<size_t>()){
            return where(path) + " must have at most " + std::to_string(max_items->get<size_t>()) + " item(s)";
        }
        auto items = schema.find("items");
        if (items != schema.end()){
            for (size_t c = 0; c < value.size(); c++){
                std::string error = validate_json_schema(*items, value[c], path + "[" + std::to_string(c) + "]");
                if (!error.empty()){
                    return error;
                }
            }
        }
    }

    if (value.is_object()){
        auto properties = schema.find("properties");
        auto required = schema.find("required");
        if (required != schema.end()){
            for (const json& name : *required){
                if (!value.contains(name.get<std::string>())){
                    return where(path) + " is missing required property \"" + name.get<std::string>() + "\"";
                }
            }
        }
        auto additional = schema.find("additionalProperties");
        bool allow_additional = additional == schema.end() || !additional->is_boolean() || additional->get<bool>();
        for (auto iter = value.begin(); iter != value.end(); ++iter){
            std::string child = path.empty() ? iter.key() : path + "." + iter.key();
            if (properties != schema.end() && properties->contains(iter.key())){
                std::string error = validate_json_schema((*properties)[iter.key()], iter.value(), child);
                if (!error.empty()){
                    return error;
                }
            }else if (!allow_additional){
                return "unknown argument \"" + child + "\"";
            }
        }
    }
    return "";
}



AgentToolDefinitions::AgentToolDefinitions(const std::string& json_text, const std::string& host){
    json data = json::parse(json_text);     //  throws json::parse_error (a std::exception)

    m_server_name = data.at("server_name").get<std::string>();
    for (const json& line : data.at("instructions")){
        if (!m_instructions.empty()){
            m_instructions += "\n";
        }
        m_instructions += line.get<std::string>();
    }

    json definitions = data.value("definitions", json::object());
    for (const json& tool : data.at("tools")){
        bool hosted = false;
        for (const json& h : tool.at("hosts")){
            hosted |= h.get<std::string>() == host;
        }
        if (!hosted){
            continue;
        }
        AgentToolDefinition definition;
        definition.name = tool.at("name").get<std::string>();
        definition.description = tool.at("description").get<std::string>();
        definition.input_schema = resolve_schema_refs(tool.at("inputSchema"), definitions);
        m_index[definition.name] = m_tools.size();
        m_tools.emplace_back(std::move(definition));
    }
}

const AgentToolDefinition* AgentToolDefinitions::find(const std::string& name) const{
    auto iter = m_index.find(name);
    return iter == m_index.end() ? nullptr : &m_tools[iter->second];
}

json AgentToolDefinitions::tools_list() const{
    json ret = json::array();
    for (const AgentToolDefinition& tool : m_tools){
        ret.push_back({
            {"name", tool.name},
            {"description", tool.description},
            {"inputSchema", tool.input_schema},
        });
    }
    return ret;
}

std::string AgentToolDefinitions::validate_arguments(const AgentToolDefinition& tool, const json& arguments) const{
    return validate_json_schema(tool.input_schema, arguments, "");
}

json AgentToolDefinitions::with_defaults(const AgentToolDefinition& tool, const json& arguments){
    json ret = arguments.is_object() ? arguments : json::object();
    auto properties = tool.input_schema.find("properties");
    if (properties == tool.input_schema.end()){
        return ret;
    }
    for (auto iter = properties->begin(); iter != properties->end(); ++iter){
        if (!ret.contains(iter.key()) && iter.value().contains("default")){
            ret[iter.key()] = iter.value()["default"];
        }
    }
    return ret;
}



}
}
