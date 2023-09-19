/*  Tera Catch On Win Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_TeraCatchOnWinOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraFarmerCatchOnWin::TeraFarmerCatchOnWin()
    : GroupOption("Catch on Win", LockWhileRunning::UNLOCKED, true)
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockWhileRunning::UNLOCKED,
        "poke-ball"
    )
    , FIX_TIME_ON_CATCH(
        "<b>Fix Clock:</b><br>Fix the time when catching so the caught date will be correct.",
        LockWhileRunning::UNLOCKED, false
    )
{
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FIX_TIME_ON_CATCH);
}




}
}
}
