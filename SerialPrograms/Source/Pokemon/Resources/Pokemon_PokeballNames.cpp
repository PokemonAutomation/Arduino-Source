/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
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
    std::string path_slugs = RESOURCE_PATH().toStdString() + "Pokemon/ItemListBalls.json";
    JsonValue json_slugs = load_json_file(path_slugs);
    JsonArray* slugs = json_slugs.get_array();
    if (slugs == nullptr){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to load resource.", std::move(path_slugs));
    }

    std::string path_disp = RESOURCE_PATH().toStdString() + "Pokemon/ItemNameDisplay.json";
    JsonValue json_disp = load_json_file(path_disp);
    JsonObject* item_disp = json_disp.get_object();
    if (item_disp == nullptr){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to load resource.", std::move(path_disp));
    }

    for (auto& item : *slugs){
        std::string* slug = item.get_string();
        if (slug == nullptr || slug->empty()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Expected non-empty string for Pokemon slug.",
                std::move(path_slugs)
            );
        }
        ordered_list.emplace_back(*slug);

        auto iter0 = item_disp->find(*slug);
        if (iter0 == item_disp->end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Unknown item slug: " + *slug,
                std::move(path_slugs)
            );
        }

        JsonObject* languages = iter0->second.get_object();
        if (languages == nullptr){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "No display names found: " + *slug,
                std::move(path_disp)
            );
        }
        auto iter1 = languages->find("eng");
        if (iter1 == languages->end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "English display not found for: " + *slug,
                std::move(path_disp)
            );
        }

        std::string* display_name = iter1->second.get_string();
        if (display_name == nullptr || display_name->empty()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Expected non-empty string for slug: *slug",
                std::move(path_disp)
            );
        }

        database[*slug].m_display_name = *display_name;
        reverse_lookup[*display_name] = *slug;
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
