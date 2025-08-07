/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_00.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


std::string AutoStory_Segment_00::name() const{
    return "00: Intro Cutscene";
}


std::string AutoStory_Segment_00::start_text() const{
    return "Start: Intro cutscene.";
}

std::string AutoStory_Segment_00::end_text() const{
    return "End: Finished cutscene.";
}

void AutoStory_Segment_00::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{

    context.wait_for_all_requests();
    env.console.log("Start Segment 00: Intro Cutscene", COLOR_ORANGE);

    checkpoint_00(env, context);

    context.wait_for_all_requests();
    env.console.log("End Segment 00: Intro Cutscene", COLOR_GREEN);
}


void checkpoint_00(SingleSwitchProgramEnvironment& env, ProControllerContext& context){


    // Mash A through intro cutscene, until the L stick button is detected
    WhiteButtonWatcher leftstick(COLOR_GREEN, WhiteButton::ButtonLStick, {0.435, 0.912, 0.046, 0.047});
    context.wait_for_all_requests();
    run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 240 * TICKS_PER_SECOND);
        },
        {leftstick}
    );
    
    // Stand up from chair and walk to left side of room
    pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(context,   0, 128, 6 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);

}




}
}
}
