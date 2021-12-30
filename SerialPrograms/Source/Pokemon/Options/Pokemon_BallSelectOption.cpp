/*  Pokemon Ball Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "Pokemon_BallSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


PokemonBallSelectData::PokemonBallSelectData(const std::vector<std::string>& slugs){
    for (const std::string& slug : slugs){
        if (slug.empty()){
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }

        using namespace NintendoSwitch::PokemonSwSh;
        const PokeballNames& data = get_pokeball_name(slug);
        const PokeballSprite* sprites = get_pokeball_sprite_nothrow(slug);
        if (sprites == nullptr){
            m_list.emplace_back(data.display_name(), QIcon());
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            m_list.emplace_back(
                data.display_name(),
                sprites->icon()
           );
        }
    }
}


PokemonBallSelect::PokemonBallSelect(
    QString label, const std::string& default_slug
)
    : PokemonBallSelectData(POKEBALL_SLUGS())
    , StringSelectOption(
        std::move(label),
        cases(),
        default_slug.empty() ? "" : get_pokeball_name(default_slug).display_name()
    )
{}

const std::string& PokemonBallSelect::slug() const{
    const QString& display = (const QString&)*this;
    return parse_pokeball_name(display);
}


}
}
