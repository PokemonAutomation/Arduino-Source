/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "PokemonSwSh_MaxLair_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


MaxLairPlayerOptions::MaxLairPlayerOptions(
    QString label,
    const LanguageSet& languages,
    bool enable_boss_ball
)
    : GroupOption(std::move(label))
    , language("<b>Game Language:</b>", languages, true)
    , normal_ball("<b>Normal Ball:</b> Ball for catching non-boss " + STRING_POKEMON + ".", "poke-ball")
    , boss_ball("<b>Boss Ball:</b> Ball for catching the boss " + STRING_POKEMON + ".", "poke-ball")
{
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(normal_ball);
    if (enable_boss_ball){
        PA_ADD_OPTION(boss_ball);
    }
}


}
}
}
