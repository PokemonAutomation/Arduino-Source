/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OliveDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_OliveActionFailedException.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_11.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





std::string AutoStory_Segment_11::name() const{
    return "11: Cortondo Gym (Bug): Gym challenge";
}

std::string AutoStory_Segment_11::start_text() const{
    return "Start: At Cortondo East Pokecenter.";
}

std::string AutoStory_Segment_11::end_text() const{
    return "End: Beat Cortondo Gym challenge. At Cortondo East Pokecenter.";
}

void AutoStory_Segment_11::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_24().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_25().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_26().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_27().run_checkpoint(env, context, options, stats);


    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_24::name() const{ return "024 - " + AutoStory_Segment_11().name(); }
std::string AutoStory_Checkpoint_24::start_text() const{ return "At Cortondo East Pokecenter";}
std::string AutoStory_Checkpoint_24::end_text() const{ return "Spoke to Cortondo Gym reception. At Cortondo West Pokecenter.";}
void AutoStory_Checkpoint_24::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_24(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_25::name() const{ return "025 - " + AutoStory_Segment_11().name(); }
std::string AutoStory_Checkpoint_25::start_text() const{ return AutoStory_Checkpoint_24().end_text();}
std::string AutoStory_Checkpoint_25::end_text() const{ return "Defeated the trainers at Olive Roll, but left Olive unmoved. Then backed out, standing in front of the Olive Roll NPC.";}
void AutoStory_Checkpoint_25::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_25(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_26::name() const{ return "026 - " + AutoStory_Segment_11().name(); }
std::string AutoStory_Checkpoint_26::start_text() const{ return AutoStory_Checkpoint_25().end_text();}
std::string AutoStory_Checkpoint_26::end_text() const{ return "Completed Olive roll gym challenge.";}
void AutoStory_Checkpoint_26::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_26(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_27::name() const{ return "027 - " + AutoStory_Segment_11().name(); }
std::string AutoStory_Checkpoint_27::start_text() const{ return AutoStory_Checkpoint_26().end_text();}
std::string AutoStory_Checkpoint_27::end_text() const{ return "At Cortondo East Pokecenter.";}
void AutoStory_Checkpoint_27::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_27(env, context, options.notif_status_update, stats);
}



void checkpoint_24(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        DirectionDetector direction;
        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0ms});
        }

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                direction.change_direction(env.program_info(), env.console, context, 2.71);
                pbf_move_left_joystick_old(context, 128, 0, 375, 100);
                direction.change_direction(env.program_info(), env.console, context, 1.26);
                pbf_move_left_joystick_old(context, 128, 0, 1750, 100);
        });        
       
        direction.change_direction(env.program_info(), env.console, context, 2.73);
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 800ms);
        pbf_wait(context, 5000ms);
        context.wait_for_all_requests();
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20000ms);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, +1}, 800ms, 160ms);
            },
            5, 5
        ); 
        // enter gym building. talk go Nemona
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        // talk to receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick_old(context, 128, 0, 450, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.26);
        pbf_move_left_joystick_old(context, 128, 0, 1600, 100);
        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
        

    });

}


void checkpoint_25(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        // section 1. align to Olive roll NPC
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 157, 0, 40);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10);
        
        // section 1.1. keep walking forward and talk to Olive roll NPC
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10000ms);
            }
        );     
        mash_button_till_overworld(env.console, context, BUTTON_A);

        context.wait_for_all_requests();

        // section 2
        pbf_move_left_joystick_old(context, 128, 0, 1300, 100);

        // section 3
        DirectionDetector direction;
        // we just hope the minimap Direction isn't covered

        direction.change_direction(env.program_info(), env.console, context, 6.0);
        pbf_move_left_joystick_old(context, 128, 0, 700, 100);

        // section 4. align to corner
        direction.change_direction(env.program_info(), env.console, context,  4.69);
        pbf_move_left_joystick(context, {0, +1}, 1200ms, 800ms);

        // section 5. battle first NPC
        direction.change_direction(env.program_info(), env.console, context,  1.485);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10000ms, 128, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Olive Roll NPC 1.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // section 6
        direction.change_direction(env.program_info(), env.console, context, 5.95);
        pbf_move_left_joystick(context, {0, +1}, 8000ms, 800ms);

        // section 7
        direction.change_direction(env.program_info(), env.console, context,  1.327);
        pbf_move_left_joystick_old(context, 128, 0, 700, 100);

        // section 8
        direction.change_direction(env.program_info(), env.console, context,  6.106);
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 800ms);

        // section 9. battle second NPC
        direction.change_direction(env.program_info(), env.console, context,  4.275);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10000ms, 128, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Olive Roll NPC 2.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // section 10. leave Olive roll
        pbf_mash_button(context, BUTTON_Y, 800ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 15, {CallbackEnum::PROMPT_DIALOG});
        wait_for_overworld(env.program_info(), env.console, context);
        enter_menu_from_overworld(env.program_info(), env.console, context, -1);

    });

}

void checkpoint_26(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        // change the time of day: close game, change time to 5:45 am.
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
        change_date(env, context, {2025, 1, 1, 5, 45, 0});
        reset_game_from_home(env.program_info(), env.console, context);

        // talk to Olive roll NPC
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10000ms, 128, 20);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // section 1
        pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);
        OliveDetector green(env.console);
        size_t MAX_ATTEMPTS_SECTION_1 = 2;
        Milliseconds duration_to_walk_for_section1 = 5200ms;
        Milliseconds push_strength_section_1 = 600ms;
        for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_1; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 4.38, duration_to_walk_for_section1, push_strength_section_1);
                // green.align_to_olive(env.program_info(), env.console, context, 4.38);
                green.walk_up_to_olive(env.program_info(), env.console, context, 4.38);
                break;
            }catch (OliveActionFailedException& e){
                if (e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // if olive is stuck, we might have pushed the olive all the way to the end. so we can try moving on.
                    break;
                }

                if (i >= MAX_ATTEMPTS_SECTION_1-1){
                    throw e;
                }                
                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED || e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE){
                    // may have walked past olive
                    pbf_move_left_joystick(context, {0, -1}, 1600ms, 400ms);
                    pbf_wait(context, 5000ms);
                    context.wait_for_all_requests();
                    duration_to_walk_for_section1 = 800ms;
                    push_strength_section_1 = 400ms;
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE
                    throw e;
                }

            }
        }        


        // section 1b. realign using fence corner
        DirectionDetector direction;
        // we just hope the minimap Direction isn't covered

        direction.change_direction(env.program_info(), env.console, context,  2.74);
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
        direction.change_direction(env.program_info(), env.console, context,  4.328);
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
        direction.change_direction(env.program_info(), env.console, context,  1.22);
        pbf_move_left_joystick_old(context, 128, 0, 75, 50);

        // section 2.1 nudge olive straight
        size_t MAX_ATTEMPTS_SECTION_2_1 = 2;
        for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_2_1; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 5.95, 400ms, 400ms);
                break;
            }catch (OliveActionFailedException& e){
                if (i >= MAX_ATTEMPTS_SECTION_2_1-1){
                    throw e;
                }

                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, {0, -1}, 2000ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 400ms, 400ms);  // move forward slight in case the olive is undetectable since it's right in front of the character
                    pbf_wait(context, 5000ms);
                    context.wait_for_all_requests();
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 1200ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();
                    break; // then move on to next section
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }                
                
            }
        }

        pbf_move_left_joystick(context, {0, -1}, 800ms, 400ms);
        green.align_to_olive(env.program_info(), env.console, context, 5.95);
        pbf_move_left_joystick(context, {0, -1}, 4000ms, 400ms);

        // section 2.2 push at angle towards outer fence
        size_t MAX_ATTEMPTS_SECTION_2_2 = 2;
        // uint16_t ticks_walked_section2_2 = 0;
        for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_2_2; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 5.8, 2000ms, 400ms);
                break;
            }catch (OliveActionFailedException& e){
                if (i >= MAX_ATTEMPTS_SECTION_2_2-1){
                    throw e;
                }

                if (e.m_fail_reason == OliveFail::OLIVE_STUCK){  // olive possibly stuck on fence
                    pbf_move_left_joystick(context, {0, -1}, 160ms, 400ms);
                    pbf_move_left_joystick(context, {-1, 0}, 800ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
                    // push olive parallel to fence
                    green.align_to_olive(env.program_info(), env.console, context, 4.28, 20);
                    green.walk_up_to_olive(env.program_info(), env.console, context, 4.28);
                    green.push_olive_forward(env.program_info(), env.console, context, 4.28, 800ms);
                    green.align_to_olive(env.program_info(), env.console, context, 4.28, 20);
                    green.walk_up_to_olive(env.program_info(), env.console, context, 4.28);
                    // back off
                    pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                    // realign using fence corner
                    direction.change_direction(env.program_info(), env.console, context,  2.74);
                    pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);
                    direction.change_direction(env.program_info(), env.console, context,  4.328);
                    pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);
                    direction.change_direction(env.program_info(), env.console, context,  1.22);
                    pbf_move_left_joystick(context, {0, +1}, 800ms, 400ms);
                }else if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, {0, -1}, 1600ms, 400ms);
                    pbf_wait(context, 5000ms);
                    context.wait_for_all_requests();                    

                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 1200ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    // continue trying to push the olive. at a different angle
                    break;
                }
            }
        }
        
        pbf_move_left_joystick(context, {0, -1}, 800ms, 400ms);
        green.align_to_olive(env.program_info(), env.console, context, 5.95);
        pbf_move_left_joystick(context, {0, -1}, 4000ms, 400ms);

        // section 2.3 push olive past first NPC
        Milliseconds duration_to_walk_for_section2_3 = 7600ms;
        size_t MAX_ATTEMPTS = 2;
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 5.95, duration_to_walk_for_section2_3, 600ms, 10);
                // green.align_to_olive(env.program_info(), env.console, context, 5.95, 10);
                green.walk_up_to_olive(env.program_info(), env.console, context, 5.95, 10);                
                break;
            }catch (OliveActionFailedException& e){
                if (e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // if olive is stuck, we might have pushed the olive all the way to the end. so we can try moving on.
                    break;
                }

                if (i >= MAX_ATTEMPTS-1){
                    throw e;
                }                
                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED || e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE){
                    // may have walked past olive
                    pbf_move_left_joystick(context, {0, -1}, 1600ms, 400ms);
                    pbf_wait(context, 5000ms);
                    context.wait_for_all_requests();                    
                    // ticks_to_walk_for_section2_3 = 500;                    
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE
                    throw e;
                }

            }
        }

        // section 2b. realign using fence corner
        direction.change_direction(env.program_info(), env.console, context,  4.5);
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
        direction.change_direction(env.program_info(), env.console, context, 5.86);
        pbf_move_left_joystick_old(context, 128, 0, 300, 50);
        direction.change_direction(env.program_info(), env.console, context,  2.76);
        pbf_move_left_joystick(context, {0, +1}, 400ms, 400ms);

        // section 3.1 push olive across the hump
        Milliseconds duration_to_walk_for_section3_1 = 2800ms;
        Milliseconds duration_walked_section3_1 = 0ms;
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                duration_walked_section3_1 = green.push_olive_forward(
                    env.program_info(),
                    env.console,
                    context,
                    1.27,
                    duration_to_walk_for_section3_1,
                    1000ms
                );
                break;
            }catch (OliveActionFailedException& e){
                // may have failed to push the olive past the hump. and walked past it
                if (i >= MAX_ATTEMPTS-1){
                    throw e;
                }

                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, {0, -1}, 2000ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 400ms, 400ms);  // move forward slight in case the olive is undetectable since it's right in front of the character
                    pbf_wait(context, 5000ms);
                    context.wait_for_all_requests();                    
                    duration_to_walk_for_section3_1 = 1600ms;
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 1200ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();
                    duration_to_walk_for_section3_1 = 1600ms;
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }
                
            }
        }

        
        // section 3.2 push olive across the hump and into fence
        Milliseconds total_duration_to_walk_for_section3 = 4400ms;
        Milliseconds duration_to_walk_for_section3_2 = 0ms;
        if (duration_walked_section3_1 < total_duration_to_walk_for_section3){
            duration_to_walk_for_section3_2 = (total_duration_to_walk_for_section3 - duration_walked_section3_1);
        }
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 1.27, duration_to_walk_for_section3_2, 1000ms);
                pbf_move_left_joystick(context, {0, -1}, 800ms, 400ms);
                green.align_to_olive(env.program_info(), env.console, context, 1.27);
                green.walk_up_to_olive(env.program_info(), env.console, context, 1.27);
                break;
            }catch (OliveActionFailedException& e){
                // may have failed to push the olive past the hump. and walked past it
                if (i >= MAX_ATTEMPTS-1){
                    throw e;
                }
                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, {0, -1}, 1600ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();                    
                    duration_to_walk_for_section3_2 += 1600ms;
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 1200ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }

            }
        }
        

        // section 3b. realign using fence.
        direction.change_direction(env.program_info(), env.console, context,  3.0);
        pbf_move_left_joystick_old(context, 128, 0, 75, 50);
        direction.change_direction(env.program_info(), env.console, context,  1.17);
        pbf_move_left_joystick(context, {0, +1}, 800ms, 400ms);

        // section 4.1 last stretch. nudge the olive
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                green.align_to_olive(env.program_info(), env.console, context, 6.0);
                green.walk_up_to_olive(env.program_info(), env.console, context, 6.0);                
                green.push_olive_forward(env.program_info(), env.console, context, 6.0, 400ms, 400ms);
                break;
            }catch (OliveActionFailedException& e){
                // may have failed to push the olive. and walked past it
                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, {0, -1}, 1600ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();                    
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                    pbf_move_left_joystick(context, {0, +1}, 800ms, 400ms);
                    pbf_wait(context, 7000ms);
                    context.wait_for_all_requests();
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }
            }
        }

        // section 4.2 past second NPC and into the finish line
        NoMinimapWatcher no_minimap(env.console, COLOR_RED, Milliseconds(500));
        size_t MAX_ATTEMPTS_SECTION_4 = 3;
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_4; i++){
                    try{
                        green.push_olive_forward(env.program_info(), env.console, context, 6.0, 2000ms, 600ms, 20, {0, 0.3, 1.0, 0.40}, false);
                        green.push_olive_forward(env.program_info(), env.console, context, 5.8, 800ms, 600ms, 20, {0, 0.3, 1.0, 0.40}, false);
                        green.push_olive_forward(env.program_info(), env.console, context, 6.0, 1600ms, 600ms, 20, {0, 0.3, 1.0, 0.40}, false);
                        green.push_olive_forward(env.program_info(), env.console, context, 6.1, 1600ms, 600ms, 20, {0, 0.3, 1.0, 0.40}, false);
                        break;
                    }catch (OliveActionFailedException& e){
                        // may have failed to push the olive. and walked past it
                        if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                            pbf_move_left_joystick(context, {0, -1}, 1600ms, 400ms);
                            pbf_wait(context, 7000ms);
                            context.wait_for_all_requests();                            
                        }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                            // try moving back and then ramming forward
                            pbf_move_left_joystick(context, {0, -1}, 400ms, 400ms);
                            pbf_move_left_joystick(context, {0, +1}, 1200ms, 400ms);
                            pbf_wait(context, 7000ms);
                            context.wait_for_all_requests();
                            // then push angled towards the right
                            green.push_olive_forward(env.program_info(), env.console, context, 5.8, 800ms, 600ms, 20, {0, 0.3, 1.0, 0.40}, false);
                        }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                            throw e;
                        }
                    }
                }                
            },
            {no_minimap}
        );
        if (ret < 0){
            throw_and_log<OliveActionFailedException>(
                env.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Failed to finish Olive roll in the last stretch.",
                env.console
            );
        }
        env.log("No minimap seen. Likely finished the Olive roll.");

        // section 8. finish olive roll
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        
        // fix the time
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);        

        enter_menu_from_overworld(env.program_info(), env.console, context, -1);

    }, false);

}

void checkpoint_27(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 128, 320ms});

    });

}





}
}
}

