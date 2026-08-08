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
