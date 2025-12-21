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
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_00().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

std::string AutoStory_Checkpoint_00::name() const{ return "000 - " + AutoStory_Segment_00().name(); }
std::string AutoStory_Checkpoint_00::start_text() const{ return "After selecting character name, style and the cutscene has started.";}
std::string AutoStory_Checkpoint_00::end_text() const{ return "Done cutscene. Stood up from chair. Walked to left side of room.";}
void AutoStory_Checkpoint_00::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_00(env, context);
}

void checkpoint_00(SingleSwitchProgramEnvironment& env, ProControllerContext& context){


    // Mash A through intro cutscene, until the L stick button is detected
    WhiteButtonWatcher leftstick(COLOR_GREEN, WhiteButton::ButtonLStick, {0.435, 0.912, 0.046, 0.047});
    context.wait_for_all_requests();
    run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 240000ms);
        },
        {leftstick}
    );
    
    // Stand up from chair and walk to left side of room
    pbf_move_left_joystick(context, 128, 255, 3000ms, 5000ms);
    pbf_move_left_joystick(context,   0, 128, 6000ms, 1000ms);

}




}
}
}
