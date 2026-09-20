/*  Tera Catch On Win Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_TeraCatchOnWinOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraFarmerCatchOnWin::TeraFarmerCatchOnWin(bool enabled)
    : GroupOption(
        "Catch on Win",
        LockMode::UNLOCK_WHILE_RUNNING,
        enabled
            ? GroupOption::EnableMode::DEFAULT_ENABLED
            : GroupOption::EnableMode::DEFAULT_DISABLED
    )
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
