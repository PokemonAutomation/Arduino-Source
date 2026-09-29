/*  Donut Powers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonLZA_DonutPowers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



std::vector<std::string> make_DONUT_POWERS_SLUGS(){
    std::vector<std::string> ret;
    for (const auto& item : DONUT_POWERS_DATABASE()){
        ret.emplace_back(item.first);
    }
    return ret;
}
const std::vector<std::string>& DONUT_POWERS_SLUGS(){
    static std::vector<std::string> database = make_DONUT_POWERS_SLUGS();
    return database;
}



struct PowerNameDatabase{
    PowerNameDatabase();

    static const PowerNameDatabase& instance(){
        static PowerNameDatabase database;
        return database;
    }

    std::map<std::string, DonutPowers> database;
    std::map<std::string, std::string> m_display_name_to_slug;
};
PowerNameDatabase::PowerNameDatabase(){
    {
        std::string path = RESOURCE_PATH() + "PokemonLZA/Donuts/flavor_powers.json";
        JsonValue json = load_json_file(path);
        JsonObject& object = json.to_object_throw(path);
        for (const auto& language_block : object){
            Language language = language_code_to_enum(language_block.first);
            const JsonObject& per_language = language_block.second.to_object_throw(path);
            for (const auto& slug : per_language){
                const JsonArray& names = slug.second.to_array_throw(path);
                if (names.empty()){
                    throw JsonParseException(path, "Expected at least one name for: " + language_block.first + " : " + slug.first);
                }
                database[slug.first].m_display_names[language] = names[0].to_string_throw();
            }
        }
        for (auto& item : database){
            auto iter = item.second.m_display_names.find(Language::English);
            if (iter == item.second.m_display_names.end()){
                throw JsonParseException(path, "English not found for: " + item.first);
            }
            item.second.m_display_name = iter->second;
            m_display_name_to_slug[iter->second] = item.first;
        }
    }
}

const DonutPowers& get_power_name(const std::string& slug){
    const std::map<std::string, DonutPowers>& database = PowerNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Donut power slug not found in database: " + slug
        );
    }
    return iter->second;
}

const std::string& parse_power_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = PowerNameDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Donut power name not found in database: " + display_name
        );
    }
    return iter->second;
}





//TODO: Make spritesheet for flavor powers
const SpriteDatabase& DONUT_POWERS_DATABASE(){
    static const SpriteDatabase database("PokemonLZA/Donuts/donut_berry_sheetSpritesheet.png", "PokemonLZA/Donuts/donut_berry_sheetSpritesheetData.json");
    return database;
}



}
}
}
