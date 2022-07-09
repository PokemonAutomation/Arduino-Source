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



JsonValue::~JsonValue(){
    clear();
}
JsonValue::JsonValue(JsonValue&& x)
//    : m_type(x.m_type)
//    , m_node(x.m_node)
{
    memcpy(this, &x, sizeof(JsonValue));
    x.m_type = JsonType::EMPTY;
}
void JsonValue::operator=(JsonValue&& x){
    if (this == &x){
        return;
    }
    clear();
    memcpy(this, &x, sizeof(JsonValue));
//    m_type = x.m_type;
//    m_node = x.m_node;
    x.m_type = JsonType::EMPTY;
}
JsonValue::JsonValue(const JsonValue& x)
    : m_type(x.m_type)
{
    switch (m_type){
    case JsonType::EMPTY:
        break;
    case JsonType::BOOLEAN:
        m_bool = x.m_bool;
        break;
    case JsonType::INTEGER:
        m_integer = x.m_integer;
        break;
    case JsonType::FLOAT:
        m_float = x.m_float;
        break;
    case JsonType::STRING:
        m_string = new std::string(*x.m_string);
        break;
    case JsonType::ARRAY:
        m_array = new JsonArray(*x.m_array);
        break;
    case JsonType::OBJECT:
        m_object = new JsonObject(*x.m_object);
        break;
    }
}
void JsonValue::operator=(const JsonValue& x){
    if (this == &x){
        return;
    }
    JsonValue tmp(x);
    operator=(std::move(tmp));
}
JsonValue JsonValue::clone() const{
    return *this;
}
void JsonValue::clear(){
    switch (m_type){
    case JsonType::STRING:
        delete m_string;
        break;
    case JsonType::ARRAY:
        delete m_array;
        break;
    case JsonType::OBJECT:
        delete m_object;
        break;
    default:;
    }
    m_type = JsonType::EMPTY;
}

JsonValue::JsonValue(bool x)
    : m_type(JsonType::BOOLEAN)
    , m_bool(x)
{}
JsonValue::JsonValue(int64_t x)
    : m_type(JsonType::INTEGER)
    , m_integer(x)
{}
JsonValue::JsonValue(double x)
    : m_type(JsonType::FLOAT)
    , m_float(x)
{}
JsonValue::JsonValue(const char* x)
    : m_type(JsonType::STRING)
    , m_string(new std::string(x))
{}
JsonValue::JsonValue(std::string x)
    : m_type(JsonType::STRING)
    , m_string(new std::string(std::move(x)))
{}
JsonValue::JsonValue(JsonArray&& x)
    : m_type(JsonType::ARRAY)
    , m_array(new JsonArray(std::move(x)))
{}
JsonValue::JsonValue(JsonObject&& x)
    : m_type(JsonType::OBJECT)
    , m_object(new JsonObject(std::move(x)))
{}
bool JsonValue::read_boolean(bool& value) const{
    if (m_type == JsonType::BOOLEAN){
        value = m_bool;
        return true;
    }
    return false;
}
bool JsonValue::read_integer(int64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = m_integer;
        return true;
    }
    return false;
}
bool JsonValue::read_integer(uint64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = m_integer;
        return true;
    }
    return false;
}
bool JsonValue::read_float(double& value) const{
    if (m_type == JsonType::INTEGER){
        value = (double)m_integer;
        return true;
    }
    if (m_type == JsonType::FLOAT){
        value = m_float;
        return true;
    }
    return false;
}
bool JsonValue::read_string(std::string& value) const{
    if (m_type == JsonType::STRING){
        value = *m_string;
        return true;
    }
    return false;
}
const std::string* JsonValue::get_string() const{
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return m_string;
}
std::string* JsonValue::get_string(){
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return m_string;
}
const JsonArray* JsonValue::get_array() const{
    return m_type == JsonType::ARRAY ? m_array : nullptr;
}
JsonArray* JsonValue::get_array(){
    return m_type == JsonType::ARRAY ? m_array : nullptr;
}
const JsonObject* JsonValue::get_object() const{
    return m_type == JsonType::OBJECT ? m_object : nullptr;
}
JsonObject* JsonValue::get_object(){
    return m_type == JsonType::OBJECT ? m_object : nullptr;
}




JsonValue parse_json(const std::string& str){
    return from_nlohmann(nlohmann::json::parse(str, nullptr, false));
}
JsonValue load_json_file(const std::string& str){
    return parse_json(file_to_string(str));
}
std::string JsonValue::dump(int indent) const{
    return to_nlohmann(*this).dump(indent);
}
void JsonValue::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}







}
