/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
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
    std::map<std::string, std::string> reverse_lookup;
};
const std::string PokeballNameDatabase::NULL_SLUG;

PokeballNameDatabase::PokeballNameDatabase(){
    QString path_slugs = RESOURCE_PATH() + "Pokemon/ItemListBalls.json";
    QString path_disp = RESOURCE_PATH() + "Pokemon/ItemNameDisplay.json";
    QJsonArray slugs = read_json_file(path_slugs).array();
    QJsonObject item_disp = read_json_file(path_disp).object();

    for (const auto& item : slugs){
        QString slug_qstr = item.toString();
        std::string slug = slug_qstr.toStdString();
        ordered_list.emplace_back(slug);

        auto iter0 = item_disp.find(slug_qstr);
        if (iter0 == item_disp.end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Unknown item slug: " + slug,
                path_slugs.toStdString()
            );
        }

        QJsonObject languages = iter0.value().toObject();
        auto iter1 = languages.find("eng");
        if (iter1 == languages.end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "English display not found for: " + slug,
                path_disp.toStdString()
            );
        }

        std::string display_name = iter1->toString().toStdString();
        database[slug].m_display_name = display_name;
        reverse_lookup[display_name] = slug;
    }
}

const PokeballNames& get_pokeball_name(const std::string& slug){
    const std::map<std::string, PokeballNames>& database = PokeballNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Pokeball slug not found in database: " + slug);
    }
    return iter->second;
}
const std::string& parse_pokeball_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = PokeballNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Pokeball name not found in database: " + display_name);
    }
    return iter->second;
}
const std::string& parse_pokeball_name_nothrow(const std::string& display_name){
    const std::map<std::string, std::string>& database = PokeballNameDatabase::instance().reverse_lookup;
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
