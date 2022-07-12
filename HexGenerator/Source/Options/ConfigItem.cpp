/*  Config Item
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Tools/Tools.h"
#include "ConfigItem.h"

namespace PokemonAutomation{


const std::string ConfigItem::JSON_TYPE     = "00-Type";
const std::string ConfigItem::JSON_LABEL    = "01-Label";

ConfigItem::ConfigItem(const JsonObject& obj)
    : m_label(obj.get_string_throw(JSON_LABEL))
{}

JsonObject ConfigItem::to_json() const{
    JsonObject root;
    root[JSON_TYPE] = type();
    root[JSON_LABEL] = m_label;
    return root;
}



std::map<std::string, OptionMaker>& OPTION_FACTORIES(){
    static std::map<std::string, OptionMaker> map;
    return map;
}
std::unique_ptr<ConfigItem> parse_option(const JsonObject& obj){
    std::string type = obj.get_string_throw(ConfigItem::JSON_TYPE);
    std::map<std::string, OptionMaker>& map = OPTION_FACTORIES();
    auto iter = map.find(type);
    if (iter == map.end()){
        throw ParseException("Unknown option type: " + type);
    }
    return iter->second(obj);
}
int register_option(const std::string& name, OptionMaker fp){
    std::map<std::string, OptionMaker>& map = OPTION_FACTORIES();
    if (!map.emplace(name, fp).second){
        throw ParseException("Duplicate option name.");
    }
    return 0;
}



}
