/*  Pokemon Pokemon Slugs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "Pokemon_PokemonSlugs.h"

namespace PokemonAutomation{
namespace Pokemon{



struct PokemonSlugDatabase{
    std::set<std::string> all_slugs;
    std::vector<std::string> national_dex;

    static PokemonSlugDatabase& instance(){
        static PokemonSlugDatabase data;
        return data;
    }
    PokemonSlugDatabase(){
        QJsonArray json = read_json_file(
            PERSISTENT_SETTINGS().resource_path + "Pokemon/Pokedex/Pokedex-National.json"
        ).array();

        for (const auto& item : json){
            QString slug_qstr = item.toString();
            if (slug_qstr.size() <= 0){
                PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
            }
            std::string slug = slug_qstr.toStdString();
            all_slugs.insert(slug);
            national_dex.emplace_back(slug);
        }
    }
};


const std::set<std::string>& ALL_POKEMON_SLUGS(){
    return PokemonSlugDatabase::instance().all_slugs;
}
const std::vector<std::string>& NATIONAL_DEX_SLUGS(){
    return PokemonSlugDatabase::instance().national_dex;
}


}
}
