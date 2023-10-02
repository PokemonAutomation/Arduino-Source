/*  Tera Catch On Win Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_TeraCatchOnWinOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraFarmerCatchOnWin::TeraFarmerCatchOnWin(bool enabled)
    : GroupOption("Catch on Win", LockMode::UNLOCK_WHILE_RUNNING, true, enabled)
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "poke-ball"
    )
    , FIX_TIME_ON_CATCH(
        "<b>Fix Clock:</b><br>Fix the time when catching so the caught date will be correct.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
{
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FIX_TIME_ON_CATCH);
}




}
}
}
