/*  JSON Object
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "JsonObject.h"
#include "JsonTools.h"

namespace PokemonAutomation{


bool JsonObject2::read_boolean(bool& value, const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_boolean(value);
}
bool JsonObject2::read_float(double& value, const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_float(value);
}
bool JsonObject2::read_string(std::string& value, const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_string(value);
}
const std::string* JsonObject2::get_string(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.get_string();
}
std::string* JsonObject2::get_string(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.get_string();
}
const JsonArray2* JsonObject2::get_array(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.get_array();
}
JsonArray2* JsonObject2::get_array(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.get_array();
}
const JsonObject2* JsonObject2::get_object(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.get_object();
}
JsonObject2* JsonObject2::get_object(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return iter->second.get_object();
}
const JsonValue2* JsonObject2::get_value(const std::string& key) const{
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return &iter->second;
}
JsonValue2* JsonObject2::get_value(const std::string& key){
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return nullptr;
    }
    return &iter->second;
}




std::string JsonObject2::dump(int indent) const{
    nlohmann::json ret;
    for (const auto& item : *this){
        ret[item.first] = to_nlohmann(item.second);
    }
    return nlohmann::json(ret).dump(indent);
}
void JsonObject2::dump(const std::string& filename, int indent) const{
    string_to_file(filename, dump(indent));
}




}
