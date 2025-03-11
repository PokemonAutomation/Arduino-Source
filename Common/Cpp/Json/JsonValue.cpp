/*  JSON Value
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "3rdParty/nlohmann/json.hpp"
#include "JsonValue.h"
#include "JsonArray.h"
#include "JsonObject.h"
#include "JsonTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


const std::string& get_typename(JsonType type){
    static const std::string NAMES[] = {
        "null",
        "Boolean",
        "Integer",
        "Float",
        "String",
        "Array",
        "Object",
    };
    return NAMES[(int)type];
}

JsonParseException::JsonParseException(
    const std::string& filename,
    JsonType expected_type,
    JsonType actual_type
){
    m_message += "Expected: " + get_typename(expected_type) + "\n";
    m_message += "Actual: " + get_typename(actual_type);
    if (!filename.empty()){
        m_message += "\n\n";
        m_message += filename;
    }
}
JsonParseException::JsonParseException(
    const std::string& filename,
    size_t size, size_t index
)
    : ParseException("Index Out-of-Bounds: size = " + std::to_string(size) + ", index = " + std::to_string(index))
{
    if (!filename.empty()){
        m_message += "\n\n";
        m_message += filename;
    }
}
JsonParseException::JsonParseException(
    const std::string& filename,
    const std::string& key
)
    : ParseException("Key not Found: " + key)
{
    if (!filename.empty()){
        m_message += "\n\n";
        m_message += filename;
    }
}
JsonParseException::JsonParseException(
    const std::string& filename,
    const std::string& key,
    JsonType expected_type,
    JsonType actual_type
)
    : ParseException("Key: " + key + "\n\n")
{
    m_message += "Expected: " + get_typename(expected_type) + "\n";
    m_message += "Actual: " + get_typename(actual_type);
    if (!filename.empty()){
        m_message += "\n\n";
        m_message += filename;
    }
}





//  RAII

JsonValue::~JsonValue(){
    clear();
}
JsonValue::JsonValue(JsonValue&& x)
    : m_type(x.m_type)
    , u(x.u)
{
    x.m_type = JsonType::EMPTY;
}
void JsonValue::operator=(JsonValue&& x){
    if (this == &x){
        return;
    }
    clear();
    m_type = x.m_type;
    u = x.u;
    x.m_type = JsonType::EMPTY;
}
JsonValue::JsonValue(const JsonValue& x)
    : m_type(x.m_type)
{
    switch (m_type){
    case JsonType::EMPTY:
        break;
    case JsonType::BOOLEAN:
        u.m_bool = x.u.m_bool;
        break;
    case JsonType::INTEGER:
        u.m_integer = x.u.m_integer;
        break;
    case JsonType::FLOAT:
        u.m_float = x.u.m_float;
        break;
    case JsonType::STRING:
        u.m_string = new std::string(*x.u.m_string);
        break;
    case JsonType::ARRAY:
        u.m_array = new JsonArray(*x.u.m_array);
        break;
    case JsonType::OBJECT:
        u.m_object = new JsonObject(*x.u.m_object);
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
        delete u.m_string;
        break;
    case JsonType::ARRAY:
        delete u.m_array;
        break;
    case JsonType::OBJECT:
        delete u.m_object;
        break;
    default:;
    }
    m_type = JsonType::EMPTY;
}



//  Constructors

JsonValue::JsonValue(bool x)
    : m_type(JsonType::BOOLEAN)
{
    u.m_bool = x;
}
JsonValue::JsonValue(int64_t x)
    : m_type(JsonType::INTEGER)
{
    u.m_integer = x;
}
JsonValue::JsonValue(double x)
    : m_type(JsonType::FLOAT)
{
    u.m_float = x;
}
JsonValue::JsonValue(const char* x)
    : m_type(JsonType::STRING)
{
    u.m_string = new std::string(x);
}
JsonValue::JsonValue(std::string x)
    : m_type(JsonType::STRING)
{
    u.m_string = new std::string(std::move(x));
}
JsonValue::JsonValue(JsonArray&& x)
    : m_type(JsonType::ARRAY)
{
    u.m_array = new JsonArray(std::move(x));
}
JsonValue::JsonValue(JsonObject&& x)
    : m_type(JsonType::OBJECT)
{
    u.m_object = new JsonObject(std::move(x));
}



//  Get with exception.

bool JsonValue::to_boolean_throw(const std::string& filename) const{
    if (m_type == JsonType::BOOLEAN){
        return u.m_bool;
    }
    throw JsonParseException(filename, JsonType::BOOLEAN, m_type);
}
int64_t JsonValue::to_integer_throw(const std::string& filename) const{
    if (m_type == JsonType::INTEGER){
        return u.m_integer;
    }
    throw JsonParseException(filename, JsonType::INTEGER, m_type);
}
double JsonValue::to_double_throw(const std::string& filename) const{
    if (m_type == JsonType::INTEGER){
        return (double)u.m_integer;
    }
    if (m_type == JsonType::FLOAT){
        return u.m_float;
    }
    throw JsonParseException(filename, JsonType::FLOAT, m_type);
}
const std::string& JsonValue::to_string_throw(const std::string& filename) const{
    if (m_type == JsonType::STRING){
        return *u.m_string;
    }
    throw JsonParseException(filename, JsonType::STRING, m_type);
}
std::string& JsonValue::to_string_throw(const std::string& filename){
    if (m_type == JsonType::STRING){
        return *u.m_string;
    }
    throw JsonParseException(filename, JsonType::STRING, m_type);
}
const JsonArray& JsonValue::to_array_throw(const std::string& filename) const{
    if (m_type == JsonType::ARRAY){
        return *u.m_array;
    }
    throw JsonParseException(filename, JsonType::ARRAY, m_type);
}
JsonArray& JsonValue::to_array_throw(const std::string& filename){
    if (m_type == JsonType::ARRAY){
        return *u.m_array;
    }
    throw JsonParseException(filename, JsonType::ARRAY, m_type);
}
const JsonObject& JsonValue::to_object_throw(const std::string& filename) const{
    if (m_type == JsonType::OBJECT){
        return *u.m_object;
    }
    throw JsonParseException(filename, JsonType::OBJECT, m_type);
}
JsonObject& JsonValue::to_object_throw(const std::string& filename){
    if (m_type == JsonType::OBJECT){
        return *u.m_object;
    }
    throw JsonParseException(filename, JsonType::OBJECT, m_type);
}



//  Get pointer.

const std::string* JsonValue::to_string() const{
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return u.m_string;
}
std::string* JsonValue::to_string(){
    if (m_type != JsonType::STRING){
        return nullptr;
    }
    return u.m_string;
}
const JsonArray* JsonValue::to_array() const{
    return m_type == JsonType::ARRAY ? u.m_array : nullptr;
}
JsonArray* JsonValue::to_array(){
    return m_type == JsonType::ARRAY ? u.m_array : nullptr;
}
const JsonObject* JsonValue::to_object() const{
    return m_type == JsonType::OBJECT ? u.m_object : nullptr;
}
JsonObject* JsonValue::to_object(){
    return m_type == JsonType::OBJECT ? u.m_object : nullptr;
}



//  Get with default.

bool JsonValue::to_boolean_default(bool default_value) const{
    if (m_type == JsonType::BOOLEAN){
        return u.m_bool;
    }
    return default_value;
}
int64_t JsonValue::to_integer_default(int64_t default_value) const{
    if (m_type == JsonType::INTEGER){
        return u.m_integer;
    }
    return default_value;
}
double JsonValue::to_double_default(double default_value) const{
    if (m_type == JsonType::INTEGER){
        return (double)u.m_integer;
    }
    if (m_type == JsonType::FLOAT){
        return u.m_float;
    }
    return default_value;
}
std::string JsonValue::to_string_default(const char* default_value) const{
    if (m_type == JsonType::STRING){
        return *u.m_string;
    }
    return default_value;
}



//  Read if matching.

bool JsonValue::read_boolean(bool& value) const{
    if (m_type == JsonType::BOOLEAN){
        value = u.m_bool;
        return true;
    }
    return false;
}
bool JsonValue::read_integer(int64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = u.m_integer;
        return true;
    }
    return false;
}
bool JsonValue::read_integer(uint64_t& value) const{
    if (m_type == JsonType::INTEGER){
        value = u.m_integer;
        return true;
    }
    return false;
}
bool JsonValue::read_float(double& value) const{
    if (m_type == JsonType::INTEGER){
        value = (double)u.m_integer;
        return true;
    }
    if (m_type == JsonType::FLOAT){
        value = u.m_float;
        return true;
    }
    return false;
}
bool JsonValue::read_string(std::string& value) const{
    if (m_type == JsonType::STRING){
        value = *u.m_string;
        return true;
    }
    return false;
}




JsonValue parse_json(const std::string& str){
    return from_nlohmann(nlohmann::json::parse(str, nullptr, false));
}
JsonValue load_json_file(const std::string& filename){
    std::string str = file_to_string(filename);
    return parse_json(str);
}
std::string JsonValue::dump(int indent) const{
    return to_nlohmann(*this).dump(indent);
}
void JsonValue::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}







}
