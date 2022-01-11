/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_PokeballNames.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokeballNameDatabase{
    PokeballNameDatabase();

    static const PokeballNameDatabase& instance(){
        static PokeballNameDatabase database;
        return database;
    }

    static const std::string NULL_SLUG;
    std::vector<std::string> ordered_list;
    std::map<std::string, PokeballNames> database;
    std::map<QString, std::string> reverse_lookup;
};
const std::string PokeballNameDatabase::NULL_SLUG;

PokeballNameDatabase::PokeballNameDatabase(){
    QJsonArray slugs = read_json_file(
        RESOURCE_PATH() + "Pokemon/ItemListBalls.json"
    ).array();

    QJsonObject item_dict = read_json_file(
        RESOURCE_PATH() + "Pokemon/ItemNameDisplay.json"
    ).object();

    for (const auto& item : slugs){
        QString slug_qstr = item.toString();
        std::string slug = slug_qstr.toStdString();
        ordered_list.emplace_back(slug);

        auto iter0 = item_dict.find(slug_qstr);
        if (iter0 == item_dict.end()){
            PA_THROW_StringException("Unknown item slug: " + slug);
        }

        QJsonObject languages = iter0.value().toObject();
        auto iter1 = languages.find("eng");
        if (iter1 == languages.end()){
            PA_THROW_StringException("English display not found for: " + slug);
        }

        QString display_name = iter1->toString();
        database[slug].m_display_name = display_name;
        reverse_lookup[display_name] = slug;
    }
}

const PokeballNames& get_pokeball_name(const std::string& slug){
    const std::map<std::string, PokeballNames>& database = PokeballNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Pokeball slug not found in database: " + slug);
    }
    return iter->second;
}
const std::string& parse_pokeball_name(const QString& display_name){
    const std::map<QString, std::string>& database = PokeballNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        PA_THROW_StringException("Pokeball name not found in database: " + display_name);
    }
    return iter->second;
}
const std::string& parse_pokeball_name_nothrow(const QString& display_name){
    const std::map<QString, std::string>& database = PokeballNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return PokeballNameDatabase::NULL_SLUG;
    }
    return iter->second;
}


const std::vector<std::string>& POKEBALL_SLUGS(){
    return PokeballNameDatabase::instance().ordered_list;
}


}
}
