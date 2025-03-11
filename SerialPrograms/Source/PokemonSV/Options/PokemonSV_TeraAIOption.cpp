/*  Tera AI Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraAIOption::TeraAIOption()
    : GroupOption("Battle AI", LockMode::UNLOCK_WHILE_RUNNING)
    , description(
        "There is no battle AI yet. It will always select the 1st move unless it is blocked by taunt, disable, torment, etc..."
    )
    , TRY_TO_TERASTILLIZE(
        "<b>Try to Terastillize:</b><br>Try to terastillize if available. Add 4s per try but greatly increase win rate.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
{
//    PA_ADD_STATIC(description);
    PA_ADD_OPTION(TRY_TO_TERASTILLIZE);
    PA_ADD_OPTION(MOVE_TABLE);
}





}
}
}
