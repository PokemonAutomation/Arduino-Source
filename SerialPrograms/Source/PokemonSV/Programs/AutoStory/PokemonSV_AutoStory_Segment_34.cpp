/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_34.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_34::name() const{
    return "34: Elite Four";
}

std::string AutoStory_Segment_34::start_text() const{
    return "Start: Beat Clavell. At Academy fly point.";
}

std::string AutoStory_Segment_34::end_text() const{
    return "End: Beat Elite Four. At Pokemon League Pokecenter.";
}

void AutoStory_Segment_34::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_86(env, context, options.notif_status_update, stats);
    checkpoint_87(env, context, options.notif_status_update, stats, options.language, options.starter_choice);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_86(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 128, 50);
        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 1.222127);
        pbf_move_left_joystick(context, 128, 0, 1100, 50);

        get_on_ride(env.program_info(), env.console, context);
        direction.change_direction(env.program_info(), env.console, context, 1.484555);

        pbf_move_left_joystick(context, 128, 0, 1506ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 703ms);
        pbf_move_left_joystick(context, 128, 0, 233ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 5098ms);
        pbf_move_left_joystick(context, 128, 0, 1000ms, 0ms);

        wait_for_overworld(env.program_info(), env.console, context);

        // marker 1    {0.429688, 0.299074}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.429688, 0.299074}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );


        // marker 2    {0.482812, 0.378704}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 50}, 
            FlyPoint::POKECENTER, 
            {0.482812, 0.378704}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );


        // marker 3    {0.638021, 0.676852}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 50}, 
            FlyPoint::POKECENTER, 
            {0.638021, 0.676852}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 60, 30, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});
        mash_button_till_overworld(env.console, context, BUTTON_A);

    });   
}

void checkpoint_87(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats, Language language, StarterChoice starter_choice){
}

std::string get_ride_pokemon_name(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language){
    enter_menu_from_overworld(env.program_info(), env.console, context, -1);

    ImageFloatBox box = {0.116223, 0.895000, 0.194915, 0.051724};
    ImageViewRGB32 image = extract_box_reference(env.console.video().snapshot(), box);

    OCR::StringMatchResult ocr_result = Pokemon::PokemonNameReader::instance().read_substring(
        env.console.logger(), language, image,
        OCR::WHITE_TEXT_FILTERS()
    );
    std::multimap<double, OCR::StringMatchData> results;
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            results.emplace(result.first, result.second);
        }
    }

    if (results.empty()){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "AutoStory_Segment_34::checkpoint_86(): Unable to read selected item. No valid results.\n" + language_warning(language),
            env.console
        );
    }

    if (results.size() > 1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "AutoStory_Segment_34::checkpoint_86(): Unable to read selected item. Ambiguous or multiple results.\n" + language_warning(language),
            env.console
        );
    }

    std::string ride_pokemon = results.begin()->second.token;

    // cout << ride_pokemon << endl;
    return ride_pokemon;
}


void checkpoint_88(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_89(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_90(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}




}
}
}
