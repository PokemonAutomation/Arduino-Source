/*  Run from Battle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
    VideoStream& stream, SwitchControllerContext& context,
    uint16_t exit_battle_time
){
    BlackScreenOverWatcher black_screen_detector;
    int ret = run_until<SwitchControllerContext>(
        stream, context,
        [exit_battle_time](SwitchControllerContext& context){
            pbf_mash_button(context, BUTTON_ZL, TICKS_PER_SECOND);
            if (exit_battle_time > TICKS_PER_SECOND){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
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
