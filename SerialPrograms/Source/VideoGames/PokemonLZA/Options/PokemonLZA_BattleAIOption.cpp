/*  Battle AI Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_BattleAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


BattleAIOption::BattleAIOption()
    : GroupOption(
        "Battle AI",
        LockMode::UNLOCK_WHILE_RUNNING,
        EnableMode::ALWAYS_ENABLED
    )
    , MODE(
        "<b>Mode:</b>",
        {
            {BattleAIMode::BlindMash,           "blind-mash",           "Blind Mash"},
            {BattleAIMode::TypeEffectiveness,   "type-effectiveness",   "Use most type-effective move."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        BattleAIMode::TypeEffectiveness
    )
    , USE_PLUS_MOVES(
        "<b>Use Plus Moves:</b><br>"
        "If enabled, it will attempt to use plus moves.<br>"
        "However, this adds a 320ms delay which may cause opponent attacks to land first.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(USE_PLUS_MOVES);
}



}
}
}
