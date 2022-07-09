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
    JsonArray2 value;
    JsonNodeArray(JsonArray2 x)
        : value(std::move(x))
    {}
};
struct JsonNodeObject : public JsonNode{
    JsonObject2 value;
    JsonNodeObject(JsonObject2 x)
        : value(std::move(x))
    {}
};




JsonValue2::~JsonValue2(){
    delete m_node;
}
JsonValue2::JsonValue2(JsonValue2&& x)
    : m_type(x.m_type)
    , m_node(x.m_node)
{
    x.m_type = JsonType::EMPTY;
    x.m_node = nullptr;
}
void JsonValue2::operator=(JsonValue2&& x){
    if (this == &x){
        return;
    }
    delete m_node;
    m_type = x.m_type;
    m_node = x.m_node;
    x.m_type = JsonType::EMPTY;
    x.m_node = nullptr;
}
JsonValue2::JsonValue2(bool x)
    : m_type(JsonType::BOOLEAN)
    , m_node(new JsonNodeBoolean(x))
{}
JsonValue2::JsonValue2(int64_t x)
    : m_type(JsonType::INTEGER)
    , m_node(new JsonNodeInteger(x))
{}
JsonValue2::JsonValue2(double x)
    : m_type(JsonType::FLOAT)
    , m_node(new JsonNodeFloat(x))
{}
JsonValue2::JsonValue2(const char* x)
    : m_type(JsonType::STRING)
    , m_node(new JsonNodeString(x))
{}
JsonValue2::JsonValue2(std::string x)
    : m_type(JsonType::STRING)
    , m_node(new JsonNodeString(std::move(x)))
{}
JsonValue2::JsonValue2(JsonArray2&& x)
    : m_type(JsonType::ARRAY)
    , m_node(new JsonNodeArray(std::move(x)))
{}
JsonValue2::JsonValue2(JsonObject2&& x)
    : m_type(JsonType::OBJECT)
    , m_node(new JsonNodeObject(std::move(x)))
{}
bool JsonValue2::read_boolean(bool& value) const{
    if (m_type == JsonType::BOOLEAN){
        value = (static_cast<const JsonNodeBoolean*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue2::read_integer(int64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = (static_cast<const JsonNodeInteger*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue2::read_integer(uint64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = (static_cast<const JsonNodeInteger*>(m_node))->value;
        return true;
    }
    return false;
}
bool JsonValue2::read_float(double& value) const{
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
bool JsonValue2::read_string(std::string& value) const{
    if (m_type == JsonType::STRING){
        value = (static_cast<const JsonNodeString*>(m_node))->value;
        return true;
    }
    return false;
}
const std::string* JsonValue2::get_string() const{
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return &static_cast<JsonNodeString*>(m_node)->value;
}
std::string* JsonValue2::get_string(){
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return &static_cast<JsonNodeString*>(m_node)->value;
}
const JsonArray2* JsonValue2::get_array() const{
    if (m_type != JsonType::ARRAY){
        return nullptr;
    }
    return &static_cast<JsonNodeArray*>(m_node)->value;
}
JsonArray2* JsonValue2::get_array(){
    if (m_type != JsonType::ARRAY){
        return nullptr;
    }
    return &static_cast<JsonNodeArray*>(m_node)->value;
}
const JsonObject2* JsonValue2::get_object() const{
    if (m_type != JsonType::OBJECT){
        return nullptr;
    }
    return &static_cast<JsonNodeObject*>(m_node)->value;
}
JsonObject2* JsonValue2::get_object(){
    if (m_type != JsonType::OBJECT){
        return nullptr;
    }
    return &static_cast<JsonNodeObject*>(m_node)->value;
}




JsonValue2 parse_json(const std::string& str){
    return from_nlohmann(nlohmann::json::parse(str));
}
std::string JsonValue2::dump(int indent) const{
    return to_nlohmann(*this).dump(indent);
}
void JsonValue2::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}







}
