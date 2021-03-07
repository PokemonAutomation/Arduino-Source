/*  Config Item
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools/Tools.h"
#include "ConfigItem.h"

namespace PokemonAutomation{


const QString ConfigItem::JSON_TYPE     = "00-Type";
const QString ConfigItem::JSON_LABEL    = "01-Label";

ConfigItem::ConfigItem(const QJsonObject& obj)
    : m_label(json_get_string(obj, JSON_LABEL))
{}

QJsonObject ConfigItem::to_json() const{
    QJsonObject root;
    root.insert(JSON_TYPE, type());
    root.insert(JSON_LABEL, m_label);
    return root;
}



std::map<QString, OptionMaker>& OPTION_FACTORIES(){
    static std::map<QString, OptionMaker> map;
    return map;
}
std::unique_ptr<ConfigItem> parse_option(const QJsonObject& obj){
    QString type = json_get_string(obj, ConfigItem::JSON_TYPE);
    std::map<QString, OptionMaker>& map = OPTION_FACTORIES();
    auto iter = map.find(type);
    if (iter == map.end()){
        throw StringException("Unknown option type: " + type);
    }
    return iter->second(obj);
}
int register_option(const QString& name, OptionMaker fp){
    std::map<QString, OptionMaker>& map = OPTION_FACTORIES();
    if (!map.emplace(name, fp).second){
        throw StringException("Duplicate option name.");
    }
    return 0;
}



}
