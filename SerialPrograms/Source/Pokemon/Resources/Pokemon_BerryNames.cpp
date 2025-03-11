/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
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
    std::map<std::string, std::string> reverse_lookup;
};
const std::string BerryNameDatabase::NULL_SLUG;

// Currently only include berries used in BDSP.
BerryNameDatabase::BerryNameDatabase()
{
    // Load a list of berry slugs in the desired order:
    // ["cheri-berry", "chesto-berry", ... ]
    std::string path_slugs = RESOURCE_PATH() + "Pokemon/ItemListBerries.json";
    JsonValue json_slugs = load_json_file(path_slugs);
    JsonArray& slugs = json_slugs.to_array_throw(path_slugs);

    // Load a map of berry slugs to berry names in all languages, e.g.:
    // {
    //      "cheri-berry": {
    //          "eng": "Cheri Berry",
    //          "fra": "Baie Ceriz",
    //          ...
    //      },
    //      ....
    // }
    std::string path_disp = RESOURCE_PATH() + "Pokemon/ItemNameDisplay.json";
    JsonValue json_disp = load_json_file(path_disp);
    JsonObject& item_disp = json_disp.to_object_throw(path_disp);

    for (auto& item : slugs){
        std::string& slug = item.to_string_throw(path_slugs);

        JsonObject& berry_name_dict = item_disp.get_object_throw(slug, path_disp);
        std::string& display_name = berry_name_dict.get_string_throw("eng", path_disp);

        ordered_list.push_back(slug);
        database[std::move(slug)].m_display_name = std::move(display_name);

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
const std::string& parse_berry_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = BerryNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Berry name not found in database: " + display_name
        );
    }
    return iter->second;
}
const std::string& parse_berry_name_nothrow(const std::string& display_name){
    const std::map<std::string, std::string>& database = BerryNameDatabase::instance().reverse_lookup;
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
