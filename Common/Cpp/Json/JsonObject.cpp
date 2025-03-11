/*  JSON Object
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "JsonObject.h"
#include "JsonTools.h"

namespace PokemonAutomation{



//  RAII

JsonObject::JsonObject(const JsonObject& x){
    for (const auto& item : x.m_data){
        m_data.emplace(item.first, item.second.clone());
    }
}
JsonObject& JsonObject::operator=(const JsonObject& x){
    JsonObject tmp(x);
    return operator=(std::move(tmp));
}
JsonObject JsonObject::clone() const{
    return *this;
}



//  Get with exception.

bool JsonObject::get_boolean_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_boolean_throw(filename);
}
int64_t JsonObject::get_integer_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_integer_throw(filename);
}
double JsonObject::get_double_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_double_throw(filename);
}
const std::string& JsonObject::get_string_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_string_throw(filename);
}
std::string& JsonObject::get_string_throw(const std::string& key, const std::string& filename){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_string_throw(filename);
}
const JsonArray& JsonObject::get_array_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_array_throw(filename);
}
JsonArray& JsonObject::get_array_throw(const std::string& key, const std::string& filename){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_array_throw(filename);
}
const JsonObject& JsonObject::get_object_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_object_throw(filename);
}
JsonObject& JsonObject::get_object_throw(const std::string& key, const std::string& filename){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second.to_object_throw(filename);
}
const JsonValue& JsonObject::get_value_throw(const std::string& key, const std::string& filename) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second;
}
JsonValue& JsonObject::get_value_throw(const std::string& key, const std::string& filename){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        throw JsonParseException(filename, key);
    }
    return iter->second;
}



//  Get pointer.

const std::string* JsonObject::get_string(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.to_string();
}
std::string* JsonObject::get_string(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.to_string();
}
const JsonArray* JsonObject::get_array(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.to_array();
}
JsonArray* JsonObject::get_array(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.to_array();
}
const JsonObject* JsonObject::get_object(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.to_object();
}
JsonObject* JsonObject::get_object(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.to_object();
}
const JsonValue* JsonObject::get_value(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return &iter->second;
}
JsonValue* JsonObject::get_value(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return &iter->second;
}



#if 0

//  Read with defaults.

bool JsonObject::to_boolean(const std::string& key, bool default_value) const{
    read_boolean(default_value, key);
    return default_value;
}
int64_t JsonObject::to_integer(const std::string& key, int64_t default_value) const{
    read_integer(default_value, key);
    return default_value;
}
double JsonObject::to_double(const std::string& key, double default_value) const{
    read_float(default_value, key);
    return default_value;
}
std::string JsonObject::to_string(const std::string& key, const char* default_value) const{
    const std::string* str = get_string(key);
    return str == nullptr ? default_value : *str;
}
#endif



//  Conditional read.

bool JsonObject::read_boolean(bool& value, const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_boolean(value);
}
bool JsonObject::read_float(double& value, const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_float(value);
}
bool JsonObject::read_string(std::string& value, const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_string(value);
}




std::string JsonObject::dump(int indent) const{
    nlohmann::json ret;
    for (const auto& item : *this){
        ret[item.first] = to_nlohmann(item.second);
    }
    return nlohmann::json(ret).dump(indent);
}
void JsonObject::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}




}
