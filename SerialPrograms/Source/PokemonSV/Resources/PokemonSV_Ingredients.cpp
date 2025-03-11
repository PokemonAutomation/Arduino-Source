/*  Pokemon Scarlet/Violet Sandwich Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonSV_Ingredients.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



std::vector<std::string> make_ALL_SANDWICH_FILLINGS_SLUGS(){
    std::vector<std::string> ret;
    for (const auto& item : SANDWICH_FILLINGS_DATABASE()){
        ret.emplace_back(item.first);
    }
    return ret;
}
const std::vector<std::string>& ALL_SANDWICH_FILLINGS_SLUGS(){
    static std::vector<std::string> database = make_ALL_SANDWICH_FILLINGS_SLUGS();
    return database;
}

std::vector<std::string> make_ALL_SANDWICH_CONDIMENTS_SLUGS(){
    std::vector<std::string> ret;
    for (const auto& item : SANDWICH_CONDIMENTS_DATABASE()){
        ret.emplace_back(item.first);
    }
    return ret;
}
const std::vector<std::string>& ALL_SANDWICH_CONDIMENTS_SLUGS(){
    static std::vector<std::string> database = make_ALL_SANDWICH_CONDIMENTS_SLUGS();
    return database;
}



struct IngredientNameDatabase{
    IngredientNameDatabase();

    static const IngredientNameDatabase& instance(){
        static IngredientNameDatabase database;
        return database;
    }

    std::map<std::string, SandwichIngredientNames> database;
    std::map<std::string, std::string> m_display_name_to_slug;
};
IngredientNameDatabase::IngredientNameDatabase(){
    {
        std::string path = RESOURCE_PATH() + "PokemonSV/Picnic/SandwichFillingOCR.json";
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
    {
        std::string path = RESOURCE_PATH() + "PokemonSV/Picnic/SandwichCondimentOCR.json";
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

const SandwichIngredientNames& get_ingredient_name(const std::string& slug){
    const std::map<std::string, SandwichIngredientNames>& database = IngredientNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Ingredient slug not found in database: " + slug
        );
    }
    return iter->second;
}

const std::string& parse_ingredient_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = IngredientNameDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Ingredient name not found in database: " + display_name
        );
    }
    return iter->second;
}






const SpriteDatabase& SANDWICH_FILLINGS_DATABASE(){
    static const SpriteDatabase database("PokemonSV/Picnic/SandwichFillingSprites.png", "PokemonSV/Picnic/SandwichFillingSprites.json");
    return database;
}

const SpriteDatabase& SANDWICH_CONDIMENTS_DATABASE(){
    static const SpriteDatabase database("PokemonSV/Picnic/SandwichCondimentSprites.png", "PokemonSV/Picnic/SandwichCondimentSprites.json");
    return database;
}



}
}
}
