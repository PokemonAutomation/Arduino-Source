/*  Egg Steps
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonTools/Resources/SpriteDatabase.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_EggSteps.h"

namespace PokemonAutomation{
namespace Pokemon{



EggStepDatabase::EggStepDatabase(const char* resource_path, const SpriteDatabase* sprites){
    std::string path_slugs = RESOURCE_PATH() + resource_path;
    JsonValue json_slugs = load_json_file(path_slugs);
    JsonObject& slugs = json_slugs.to_object_throw(path_slugs);

    const std::map<std::string, size_t>& SLUGS_TO_NATIONAL_DEX = Pokemon::SLUGS_TO_NATIONAL_DEX();

    std::map<size_t, std::pair<std::string, uint16_t>> nat_id_to_steps;
    for (const auto& slug : slugs){
        auto iter = SLUGS_TO_NATIONAL_DEX.find(slug.first);
        if (iter == SLUGS_TO_NATIONAL_DEX.end()){
            global_logger_tagged().log("Unknown " + Pokemon::STRING_POKEMON + "slug: " + slug.first);
            continue;
        }

        auto ret = nat_id_to_steps.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(iter->second),
            std::forward_as_tuple(iter->first, (uint16_t)slug.second.to_integer_throw(path_slugs))
        );
        if (!ret.second){
            global_logger_tagged().log("Duplicate " + Pokemon::STRING_POKEMON + " nat-dex ID: " + std::to_string(iter->second));
        }

        m_slug_to_steps[slug.first] = (uint16_t)slug.second.to_integer_throw(path_slugs);
    }

    for (const auto& item : nat_id_to_steps){
        const std::string& slug = item.second.first;
        std::string display_name = Pokemon::get_pokemon_name(slug).display_name();
        display_name += " (" + tostr_u_commas(item.second.second) + " steps)";

        const SpriteDatabase::Sprite* sprite = sprites == nullptr
            ? nullptr
            : sprites->get_nothrow(slug);

        if (sprite == nullptr){
            m_stringselect_database.add_entry(StringSelectEntry(slug, display_name));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            m_stringselect_database.add_entry(StringSelectEntry(slug, display_name, sprite->icon));
        }
    }
}


size_t EggStepDatabase::step_count(const std::string& slug) const{
    auto iter = m_slug_to_steps.find(slug);
    if (iter == m_slug_to_steps.end()){
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            std::string("Invalid ") + Pokemon::STRING_POKEMON + " slug: " + slug
        );
    }
    return iter->second;
}



}
}
