/*  Locations Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Globals.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "PokemonFRLG_LocationsDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


std::vector<std::pair<std::string, std::string>> load_location_slug_json_list(const char* json_path){
    std::string path = RESOURCE_PATH() + json_path;
    JsonValue json = load_json_file(path);
    JsonObject& obj = json.to_object_throw();

    std::vector<std::pair<std::string, std::string>> list;
    for (auto& item : obj){
        std::string slug = item.first;
        std::string name = item.second.to_string_throw();
        std::pair<std::string, std::string> pair = {slug, name};
        list.emplace_back(pair);
    }
    return list;
}

StringSelectDatabase make_locations_database(const std::vector<std::pair<std::string, std::string>>& slug_pairs){
    StringSelectDatabase database;
    for (const std::pair<std::string, std::string>& slug_pair : slug_pairs){
        database.add_entry(StringSelectEntry(
            slug_pair.first,
            slug_pair.second
        ));
    }
    return database;
}

StringSelectDatabase make_locations_database(const char* json_file_slugs){
    return make_locations_database(load_location_slug_json_list(json_file_slugs));
}



}
}
}
