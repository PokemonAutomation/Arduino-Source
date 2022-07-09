/*  JSON Value
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "3rdParty/nlohmann/json.hpp"
#include "JsonValue.h"
#include "JsonArray.h"
#include "JsonObject.h"
#include "JsonTools.h"

namespace PokemonAutomation{




struct JsonNode{
    virtual ~JsonNode() = default;
};
struct JsonNodeBoolean : public JsonNode{
    bool value;
    JsonNodeBoolean(bool x)
        : value(x)
    {}
};
struct JsonNodeInteger : public JsonNode{
    int64_t value;
    JsonNodeInteger(int64_t x)
        : value(x)
    {}
};
struct JsonNodeFloat : public JsonNode{
    double value;
    JsonNodeFloat(double x)
        : value(x)
    {}
};
struct JsonNodeString : public JsonNode{
    std::string value;
    JsonNodeString(std::string x)
        : value(std::move(x))
    {}
};
struct JsonNodeArray : public JsonNode{
    JsonArray value;
    JsonNodeArray(JsonArray x)
        : value(std::move(x))
    {}
};
struct JsonNodeObject : public JsonNode{
    JsonObject value;
    JsonNodeObject(JsonObject x)
        : value(std::move(x))
    {}
};




JsonValue::~JsonValue(){
    delete m_node;
}
JsonValue::JsonValue(JsonValue&& x)
    : m_type(x.m_type)
    , m_node(x.m_node)
{
    x.m_type = JsonType::EMPTY;
    x.m_node = nullptr;
}
void JsonValue::operator=(JsonValue&& x){
    if (this == &x){
        return;
    }
    delete m_node;
    m_type = x.m_type;
    m_node = x.m_node;
    x.m_type = JsonType::EMPTY;
    x.m_node = nullptr;
}
JsonValue::JsonValue(bool x)
    : m_type(JsonType::BOOLEAN)
    , m_node(new JsonNodeBoolean(x))
{}
JsonValue::JsonValue(int64_t x)
    : m_type(JsonType::INTEGER)
    , m_node(new JsonNodeInteger(x))
{}
JsonValue::JsonValue(double x)
    : m_type(JsonType::FLOAT)
    , m_node(new JsonNodeFloat(x))
{}
JsonValue::JsonValue(const char* x)
    : m_type(JsonType::STRING)
    , m_node(new JsonNodeString(x))
{}
JsonValue::JsonValue(std::string x)
    : m_type(JsonType::STRING)
    , m_node(new JsonNodeString(std::move(x)))
{}
JsonValue::JsonValue(JsonArray&& x)
    : m_type(JsonType::ARRAY)
    , m_node(new JsonNodeArray(std::move(x)))
{}
JsonValue::JsonValue(JsonObject&& x)
    : m_type(JsonType::OBJECT)
    , m_node(new JsonNodeObject(std::move(x)))
{}
bool JsonValue::read_boolean(bool& value) const{
    if (m_type == JsonType::BOOLEAN){
        value = (static_cast<const JsonNodeBoolean*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue::read_integer(int64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = (static_cast<const JsonNodeInteger*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue::read_integer(uint64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = (static_cast<const JsonNodeInteger*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue::read_float(double& value) const{
    if (m_type == JsonType::INTEGER){
        value = (double)(static_cast<const JsonNodeInteger*>(m_node))->value;
        return true;
    }
    if (m_type == JsonType::FLOAT){
        value = (static_cast<const JsonNodeFloat*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue::read_string(std::string& value) const{
    if (m_type == JsonType::STRING){
        value = (static_cast<const JsonNodeString*>(m_node))->value;
        return true;
    }
    return false;
}
const std::string* JsonValue::get_string() const{
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return &static_cast<JsonNodeString*>(m_node)->value;
}
std::string* JsonValue::get_string(){
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return &static_cast<JsonNodeString*>(m_node)->value;
}
const JsonArray* JsonValue::get_array() const{
    if (m_type != JsonType::ARRAY){
        return nullptr;
    }
    return &static_cast<JsonNodeArray*>(m_node)->value;
}
JsonArray* JsonValue::get_array(){
    if (m_type != JsonType::ARRAY){
        return nullptr;
    }
    return &static_cast<JsonNodeArray*>(m_node)->value;
}
const JsonObject* JsonValue::get_object() const{
    if (m_type != JsonType::OBJECT){
        return nullptr;
    }
    return &static_cast<JsonNodeObject*>(m_node)->value;
}
JsonObject* JsonValue::get_object(){
    if (m_type != JsonType::OBJECT){
        return nullptr;
    }
    return &static_cast<JsonNodeObject*>(m_node)->value;
}




JsonValue parse_json(const std::string& str){
    return from_nlohmann(nlohmann::json::parse(str));
}
std::string JsonValue::dump(int indent) const{
    return to_nlohmann(*this).dump(indent);
}
void JsonValue::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}







}
