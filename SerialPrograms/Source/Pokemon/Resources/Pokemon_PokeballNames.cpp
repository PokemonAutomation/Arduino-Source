/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
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
    std::string path_slugs = RESOURCE_PATH() + "Pokemon/ItemListBalls.json";
    JsonValue json_slugs = load_json_file(path_slugs);
    JsonArray& slugs = json_slugs.to_array_throw(path_slugs);

    std::string path_disp = RESOURCE_PATH() + "Pokemon/ItemNameDisplay.json";
    JsonValue json_disp = load_json_file(path_disp);
    JsonObject& item_disp = json_disp.to_object_throw(path_disp);

    for (auto& item : slugs){
        std::string& slug = item.to_string_throw(path_slugs);
        ordered_list.emplace_back(slug);

        JsonObject* languages = item_disp.get_object(slug);
        std::string* display_name;
        if (languages != nullptr){
            //  TODO: REMOVE: Display names for PLA balls.
            display_name = &languages->get_string_throw("eng", path_disp);
        }else{
            display_name = &slug;
        }

        database[slug].m_display_name = *display_name;
        reverse_lookup[*display_name] = slug;
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
