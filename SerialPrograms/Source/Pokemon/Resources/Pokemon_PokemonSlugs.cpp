/*  Pokemon Pokemon Slugs
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "Pokemon_PokemonSlugs.h"

namespace PokemonAutomation{
namespace Pokemon{



struct PokemonSlugDatabase{
    std::set<std::string> all_slugs;
    std::vector<std::string> national_dex;
    std::map<std::string, size_t> slugs_to_dex;

    static PokemonSlugDatabase& instance(){
        static PokemonSlugDatabase data;
        return data;
    }
    PokemonSlugDatabase(){
        std::string path = RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-National.json";
        JsonValue json = load_json_file(path);
        JsonArray& slugs = json.to_array_throw(path);

        for (auto& item : slugs){
            std::string& slug = item.to_string_throw(path);
            all_slugs.insert(slug);
            national_dex.emplace_back(slug);
            slugs_to_dex[std::move(slug)] = national_dex.size();
        }
    }
};


const std::set<std::string>& ALL_POKEMON_SLUGS(){
    return PokemonSlugDatabase::instance().all_slugs;
}
const std::vector<std::string>& NATIONAL_DEX_SLUGS(){
    return PokemonSlugDatabase::instance().national_dex;
}
const std::map<std::string, size_t>& SLUGS_TO_NATIONAL_DEX(){
    return PokemonSlugDatabase::instance().slugs_to_dex;
}


}
}
