/*  Pokemon Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_PokemonNames.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonNameDatabase{
    PokemonNameDatabase();
    static const PokemonNameDatabase& instance(){
        static PokemonNameDatabase database;
        return database;
    }

    static const std::string NULL_SLUG;
    std::map<std::string, PokemonNames> m_slug_to_data;
    std::map<std::string, std::string> m_display_name_to_slug;
};
const std::string PokemonNameDatabase::NULL_SLUG;

PokemonNameDatabase::PokemonNameDatabase(){
    std::string path = RESOURCE_PATH().toStdString() + "Pokemon/PokemonNameDisplay.json";
    JsonValue json = load_json_file(path);
    JsonObject& displays = json.get_object_throw(path);

    for (auto& item0 : displays){
        const std::string& slug = item0.first;

        JsonObject& names = item0.second.get_object_throw(path);
        PokemonNames data;
        for (auto& item1 : names){
            std::string& name = item1.second.get_string_throw(path);
            data.m_display_names[language_code_to_enum(item1.first)] = std::move(name);
        }

        //  Display name for English.
        auto iter2 = data.m_display_names.find(Language::English);
        if (iter2 == data.m_display_names.end()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Missing English translation: " + slug,
                std::move(path)
            );
        }
        data.m_display_name = iter2->second;

        m_display_name_to_slug.emplace(data.m_display_name, slug);
        m_slug_to_data.emplace(slug, std::move(data));
    }
}


const std::string& PokemonNames::display_name(Language language) const{
    auto iter = m_display_names.find(language);
    if (iter == m_display_names.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No data loaded for this language.");
    }
    return iter->second;
}



const PokemonNames& get_pokemon_name(const std::string& slug){
    const std::map<std::string, PokemonNames>& database = PokemonNameDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Pokemon slug not found in database: " + slug);
    }
    return iter->second;
}
const PokemonNames* get_pokemon_name_nothrow(const std::string& slug){
    const std::map<std::string, PokemonNames>& database = PokemonNameDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        return nullptr;
    }
    return &iter->second;
}
const std::string& parse_pokemon_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = PokemonNameDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Display name not found in database: " + display_name);
    }
    return iter->second;
}
const std::string& parse_pokemon_name_nothrow(const std::string& display_name){
    const std::map<std::string, std::string>& database = PokemonNameDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return PokemonNameDatabase::NULL_SLUG;
    }
    return iter->second;
}

std::vector<std::string> load_pokemon_slug_json_list(const char* json_path){
    std::string path = RESOURCE_PATH().toStdString() + json_path;
    JsonValue json = load_json_file(path);
    JsonArray& array = json.get_array_throw();

    std::vector<std::string> list;
    for (auto& item : array){
        std::string& slug = item.get_string_throw();
        if (slug.empty()){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Expected non-empty string for Pokemon slug.",
                std::move(path)
            );
        }
        list.emplace_back(std::move(slug));
    }
    return list;
}

}
}
