/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_07.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_07::name() const{
    return "07: Go to Mesagoza South";
}

std::string AutoStory_Segment_07::start_text() const{
    return "Start: At Los Platos Pokecenter. Cleared Let's go tutorial.";
}

std::string AutoStory_Segment_07::end_text() const{
    return "End: At Mesagoza South Pokecenter.";
}

void AutoStory_Segment_07::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);

    checkpoint_12(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);

}

void checkpoint_12(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    // reset rate: ~25%. 12 resets out of 52. 
    // resets due to: getting attacked by wild pokemon, either from behind, 
    // or when lead pokemon not strong enough to clear them with Let's go
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
        context.wait_for_all_requests();

        // re-orient camera
        pbf_press_button(context, BUTTON_L, 20, 20);
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_while_clear_front_path(env.program_info(), env.console, context, 35);

                // place the marker elsewhere
                realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 50);

                DirectionDetector direction;
                direction.change_direction(info, env.console, context, 0);
                walk_forward_while_clear_front_path(info, env.console, context, 3300, 0, 125, 125);

                // check we're not still at the Los Platos Pokecenter.
                confirm_no_overlapping_flypoint(info, env.console, context);
                // not stuck at Los Platos Pokecenter
                pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
                press_Bs_to_back_to_overworld(info, env.console, context, 7);

                direction.change_direction(info, env.console, context, 0.29);
                walk_forward_while_clear_front_path(info, env.console, context, 1200, 0, 125, 125);
                direction.change_direction(info, env.console, context, 0.61);
                walk_forward_while_clear_front_path(info, env.console, context, 1200, 0, 125, 125);

                fly_to_overlapping_flypoint(info, env.console, context);
            }
        );

        env.console.log("Reached Mesagoza (South) Pokecenter.");

    });

}



}
}
}
