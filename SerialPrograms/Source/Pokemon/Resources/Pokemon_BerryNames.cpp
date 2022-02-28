/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_BerryNames.h"

namespace PokemonAutomation{
namespace Pokemon{


struct BerryNameDatabase{
    BerryNameDatabase();

    static const BerryNameDatabase& instance(){
        static BerryNameDatabase database;
        return database;
    }

    static const std::string NULL_SLUG;
    std::vector<std::string> ordered_list;
    std::map<std::string, BerryNames> database;
    std::map<QString, std::string> reverse_lookup;
};
const std::string BerryNameDatabase::NULL_SLUG;

// Currently only include berries used in BDSP.
BerryNameDatabase::BerryNameDatabase()
{
    // Load a list of berry slugs in the desired order:
    // ["cheri-berry", "chesto-berry", ... ]
    QString path_slugs = RESOURCE_PATH() + "Pokemon/ItemListBerries.json";
    const QJsonArray slug_list = read_json_file(path_slugs).array();

    // Load a map of berry slugs to berry names in all languages, e.g.:
    // {
    //      "cheri-berry": {
    //          "eng": "Cheri Berry",
    //          "fra": "Baie Ceriz",
    //          ...
    //      },
    //      ....
    // }
    QString path_disp = RESOURCE_PATH() + "Pokemon/ItemNameDisplay.json";
    const QJsonObject item_disp = read_json_file(path_disp).object();

    for (auto iter = slug_list.begin(); iter != slug_list.end(); ++iter){
        const QString slug = iter->toString();
        const auto berry_name_dict_iter = item_disp.find(slug);
        if (berry_name_dict_iter == item_disp.end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Fail to find berry " + slug.toStdString() + " in ItemNameDisplay.json",
                path_disp.toStdString()
            );
        }
        const auto berry_name_dict = berry_name_dict_iter->toObject();
        auto berry_eng_name_iter = berry_name_dict.find("eng");
        if (berry_eng_name_iter == berry_name_dict.end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Fail to find English display name for berry " + slug.toStdString(),
                path_disp.toStdString()
            );
        }

        QString display_name = berry_eng_name_iter->toString();
        const std::string slug_str = slug.toStdString();
        ordered_list.push_back(slug_str);
        database[slug_str].m_display_name = display_name;

        // std::cout << "Berry: " << slug_str << " -> " << display_name.toStdString() << std::endl;
    }

    for (const auto& item : database){
        reverse_lookup[item.second.m_display_name] = item.first;
    }
}

const BerryNames& get_berry_name(const std::string& slug){
    const std::map<std::string, BerryNames>& database = BerryNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Berry slug not found in database: " + slug
        );
    }
    return iter->second;
}
const std::string& parse_berry_name(const QString& display_name){
    const std::map<QString, std::string>& database = BerryNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Berry name not found in database: " + display_name.toStdString()
        );
    }
    return iter->second;
}
const std::string& parse_berry_name_nothrow(const QString& display_name){
    const std::map<QString, std::string>& database = BerryNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return BerryNameDatabase::NULL_SLUG;
    }
    return iter->second;
}


const std::vector<std::string>& BERRY_SLUGS(){
    return BerryNameDatabase::instance().ordered_list;
}


}
}
