/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_35.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_35::name() const{
    return "35: Area Zero Gate, Station 1";
}

std::string AutoStory_Segment_35::start_text() const{
    return "Start: Beat Nemona, Penny, and Arven. At Los Platos Pokecenter.";
}

std::string AutoStory_Segment_35::end_text() const{
    return "End: ";
}

void AutoStory_Segment_35::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_93().run_checkpoint(env, context, options, stats);


    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_93::name() const{ return "093 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_93::start_text() const{ return "Beat Arven. At Los Platos Pokecenter.";}
std::string AutoStory_Checkpoint_93::end_text() const{ return "Entered Area Zero Gate. Flew down to Area Zero from the platform.";}
void AutoStory_Checkpoint_93::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_93(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_94::name() const{ return "094 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_94::start_text() const{ return AutoStory_Checkpoint_93().end_text();}
std::string AutoStory_Checkpoint_94::end_text() const{ return "At Area Zero Station 1.";}
void AutoStory_Checkpoint_94::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_94(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_95::name() const{ return "095 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_95::start_text() const{ return "";}
std::string AutoStory_Checkpoint_95::end_text() const{ return "";}
void AutoStory_Checkpoint_95::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_95(env, context, options.notif_status_update, stats);
}

// std::string AutoStory_Checkpoint_96::name() const{ return "096 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_96::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_96::end_text() const{ return "";}
// void AutoStory_Checkpoint_96::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_96(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_97::name() const{ return "097 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_97::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_97::end_text() const{ return "";}
// void AutoStory_Checkpoint_97::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_97(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_98::name() const{ return "098 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_98::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_98::end_text() const{ return "";}
// void AutoStory_Checkpoint_98::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_98(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_99::name() const{ return "099 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_99::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_99::end_text() const{ return "";}
// void AutoStory_Checkpoint_99::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_99(env, context, options.notif_status_update, stats);
// }

void checkpoint_93(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // fly to Medali West
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 95, 0, 170}, FlyPoint::POKECENTER);

        // marker 1     {0.580729, 0.286111}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.580729, 0.286111}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 2    {0.475, 0.4}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.475, 0.4}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 3    {0.473958, 0.260185}      {0.479687, 0.250926}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 255, 30}, 
            FlyPoint::POKECENTER, 
            {0.479687, 0.250926}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 4     {0.425, 0.289815}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.425, 0.289815}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 5     {0.465104, 0.292593}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 0, 20}, 
            FlyPoint::POKECENTER, 
            {0.465104, 0.292593}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 6     {0.439583, 0.274074}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_OUT, 255, 255, 20}, 
            FlyPoint::POKECENTER, 
            {0.439583, 0.274074}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 120, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX});
        pbf_move_left_joystick(context, 128, 0, 100, 50);
        pbf_move_left_joystick(context, 255, 70, 300, 50);
        mash_button_till_overworld(env.console, context, BUTTON_A);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 120, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX, CallbackEnum::PROMPT_DIALOG});

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);
        mash_button_till_overworld(env.console, context, BUTTON_A); // prompt, black dialog. random A press

    }); 

}

void checkpoint_94(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
       

    });     
}

void checkpoint_95(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

// void checkpoint_96(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_97(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_98(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_99(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }




}
}
}
