/*  Run from Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_RunFromBattle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



bool run_from_battle(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds exit_battle_time
){
    BlackScreenOverWatcher black_screen_detector;
    int ret = run_until<ProControllerContext>(
        stream, context,
        [exit_battle_time](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_ZL, 1000ms);
            if (exit_battle_time > 1000ms){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - 1000ms);
            }
        },
        {{black_screen_detector}}
    );
    if (ret < 0){
        stream.log("Timed out waiting for end of battle. Are you stuck in the battle?", COLOR_RED);
        return false;
    }
    pbf_wait(context, TICKS_PER_SECOND);
    context.wait_for_all_requests();
    return true;
}



}
}
}
