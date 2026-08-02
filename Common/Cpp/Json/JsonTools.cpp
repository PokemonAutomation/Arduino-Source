/*  JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/FileIO.h"
#include "JsonTools.h"
#include "JsonArray.h"
#include "JsonObject.h"

namespace PokemonAutomation{



void string_to_file(const std::string& filename, const std::string& str){
    std::string json_out = "\xef\xbb\xbf";
    //  Convert to CRLF.
    char previous = 0;
    for (char ch : str){
        if (ch == '\n' && previous != '\r'){
            json_out += '\r';
        }
        json_out += ch;
        previous = ch;
    }

    FileIO file;
    if (!file.open(filename, FileMode::WRITE)){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to create file.", filename);
    }
    if (file.write(json_out.c_str(), json_out.size()) != json_out.size()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to write file.", filename);
    }
    file.close();
}


std::string file_to_string(const std::string& filename){
    std::string ret;
    if (!file_to_string(filename, ret)){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to open file.", filename);
    }
    return ret;
}


bool file_to_string(const std::string& filename, std::string& content){
    FileIO file;
    if (!file.open(filename, FileMode::READ)){
        return false;
    }

    char buffer[4096];

    std::string ret;
    while (true){
        size_t actual = file.read(buffer, sizeof(buffer));
        ret += std::string(buffer, actual);
        if (actual < sizeof(buffer)){
            break;
        }
    }
    content = std::move(ret);
    return true;
}




JsonValue from_nlohmann(const nlohmann::json& json){
    if (json.is_null()){
        return JsonValue();
    }
    if (json.is_boolean()){
        return JsonValue((bool)json);
    }
    if (json.is_number_integer()){
        return JsonValue((int64_t)json);
    }
    if (json.is_number()){
        return JsonValue((double)json);
    }
    if (json.is_string()){
        return JsonValue((std::string)json);
    }
    if (json.is_array()){
        JsonArray array;
        size_t size = json.size();
        for (size_t c = 0; c < size; c++){
            array.push_back(from_nlohmann(json[c]));
        }
        return array;
    }
    if (json.is_object()){
        JsonObject object;
        for (auto it = json.begin(); it != json.end(); ++it){
            object[it.key()] = from_nlohmann(it.value());
        }
        return object;
    }
    return JsonValue();
}
nlohmann::json to_nlohmann(const JsonValue& json){
    if (json.is_null()){
        return nlohmann::json();
    }
    {
        bool value;
        if (json.read_boolean(value)){
            return value;
        }
    }
    {
        int64_t value;
        if (json.read_integer(value)){
            return value;
        }
    }
    {
        double value;
        if (json.read_float(value)){
            return value;
        }
    }
    {
        std::string value;
        if (json.read_string(value)){
            return value;
        }
    }
    if (json.is_array()){
        nlohmann::json::array_t ret;
        for (const auto& item : *json.to_array()){
            ret.emplace_back(to_nlohmann(item));
        }
        return ret;
    }
    if (json.is_object()){
        nlohmann::json ret;
        for (const auto& item : *json.to_object()){
            ret[item.first] = to_nlohmann(item.second);
        }
        return ret;
    }
    return nlohmann::json();
}





}
