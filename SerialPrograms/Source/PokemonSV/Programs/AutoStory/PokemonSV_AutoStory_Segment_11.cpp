/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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

    checkpoint_24(env, context, options.notif_status_update, stats);
    checkpoint_25(env, context, options.notif_status_update, stats);
    checkpoint_26(env, context, options.notif_status_update, stats);
    checkpoint_27(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

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
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                direction.change_direction(env.program_info(), env.console, context, 2.71);
                pbf_move_left_joystick(context, 128, 0, 375, 100);
                direction.change_direction(env.program_info(), env.console, context, 1.26);
                pbf_move_left_joystick(context, 128, 0, 1750, 100);                
        });        
       
        direction.change_direction(env.program_info(), env.console, context, 2.73);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        pbf_wait(context, 5 * TICKS_PER_SECOND);
        context.wait_for_all_requests();
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 0, 100, 20);
            },
            5, 5
        ); 
        // enter gym building. talk go Nemona
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        // talk to receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick(context, 128, 0, 450, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.26);
        pbf_move_left_joystick(context, 128, 0, 1600, 100);
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
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
            }
        );     
        mash_button_till_overworld(env.console, context, BUTTON_A);

        context.wait_for_all_requests();

        // section 2
        pbf_move_left_joystick(context, 128, 0, 1300, 100);

        // section 3
        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 6.0);
        pbf_move_left_joystick(context, 128, 0, 700, 100);

        // section 4. align to corner
        direction.change_direction(env.program_info(), env.console, context,  4.69);
        pbf_move_left_joystick(context, 128, 0, 150, 100);

        // section 5. battle first NPC
        direction.change_direction(env.program_info(), env.console, context,  1.485);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10, 128, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Olive Roll NPC 1.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // section 6
        direction.change_direction(env.program_info(), env.console, context, 5.95);
        pbf_move_left_joystick(context, 128, 0, 1000, 100);

        // section 7
        direction.change_direction(env.program_info(), env.console, context,  1.327);
        pbf_move_left_joystick(context, 128, 0, 700, 100);

        // section 8
        direction.change_direction(env.program_info(), env.console, context,  6.106);
        pbf_move_left_joystick(context, 128, 0, 200, 100);

        // section 9. battle second NPC
        direction.change_direction(env.program_info(), env.console, context,  4.275);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10, 128, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Olive Roll NPC 2.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // section 10. leave Olive roll
        pbf_mash_button(context, BUTTON_Y, 100);
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
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10, 128, 20);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // section 1
        pbf_move_left_joystick(context, 128, 0, 400, 50);
        OliveDetector green(env.console);
        size_t MAX_ATTEMPTS_SECTION_1 = 2;
        uint16_t ticks_to_walk_for_section1 = 650;
        uint16_t push_strength_section_1 = 75;
        for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_1; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 4.38, ticks_to_walk_for_section1, push_strength_section_1);
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
                    pbf_move_left_joystick(context, 128, 255, 200, 50);
                    pbf_wait(context, 5 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                    ticks_to_walk_for_section1 = 100;
                    push_strength_section_1 = 50;               
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE
                    throw e;
                }

            }
        }        


        // section 1b. realign using fence corner
        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context,  2.74);
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        direction.change_direction(env.program_info(), env.console, context,  4.328);
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        direction.change_direction(env.program_info(), env.console, context,  1.22);
        pbf_move_left_joystick(context, 128, 0, 75, 50);

        // section 2.1 nudge olive straight
        size_t MAX_ATTEMPTS_SECTION_2_1 = 2;
        for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_2_1; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 5.95, 50, 50);
                break;
            }catch (OliveActionFailedException& e){
                if (i >= MAX_ATTEMPTS_SECTION_2_1-1){
                    throw e;
                }

                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, 128, 255, 250, 50);
                    pbf_move_left_joystick(context, 128, 0, 50, 50);  // move forward slight in case the olive is undetectable since it's right in front of the character
                    pbf_wait(context, 5 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, 128, 255, 50, 50);
                    pbf_move_left_joystick(context, 128, 0, 150, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                    break; // then move on to next section
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }                
                
            }
        }

        pbf_move_left_joystick(context, 128, 255, 100, 50);
        green.align_to_olive(env.program_info(), env.console, context, 5.95);
        pbf_move_left_joystick(context, 128, 255, 500, 50);        

        // section 2.2 push at angle towards outer fence
        size_t MAX_ATTEMPTS_SECTION_2_2 = 2;
        // uint16_t ticks_walked_section2_2 = 0;
        for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_2_2; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 5.8, 250, 50);
                break;
            }catch (OliveActionFailedException& e){
                if (i >= MAX_ATTEMPTS_SECTION_2_2-1){
                    throw e;
                }

                if (e.m_fail_reason == OliveFail::OLIVE_STUCK){  // olive possibly stuck on fence
                    pbf_move_left_joystick(context, 128, 255, 20, 50);
                    pbf_move_left_joystick(context, 0, 128, 100, 50);
                    pbf_move_left_joystick(context, 128, 0, 200, 50);
                    // push olive parallel to fence
                    green.align_to_olive(env.program_info(), env.console, context, 4.28, 20);
                    green.walk_up_to_olive(env.program_info(), env.console, context, 4.28);
                    green.push_olive_forward(env.program_info(), env.console, context, 4.28, 100);
                    green.align_to_olive(env.program_info(), env.console, context, 4.28, 20);
                    green.walk_up_to_olive(env.program_info(), env.console, context, 4.28);
                    // back off
                    pbf_move_left_joystick(context, 128, 255, 50, 50);
                    // realign using fence corner
                    direction.change_direction(env.program_info(), env.console, context,  2.74);
                    pbf_move_left_joystick(context, 128, 0, 400, 50);
                    direction.change_direction(env.program_info(), env.console, context,  4.328);
                    pbf_move_left_joystick(context, 128, 0, 400, 50);
                    direction.change_direction(env.program_info(), env.console, context,  1.22);
                    pbf_move_left_joystick(context, 128, 0, 100, 50);                    
                }else if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, 128, 255, 200, 50);
                    pbf_wait(context, 5 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();                    

                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, 128, 255, 50, 50);
                    pbf_move_left_joystick(context, 128, 0, 150, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    // continue trying to push the olive. at a different angle
                    break;
                }
            }
        }
        
        pbf_move_left_joystick(context, 128, 255, 100, 50);
        green.align_to_olive(env.program_info(), env.console, context, 5.95);
        pbf_move_left_joystick(context, 128, 255, 500, 50);

        // section 2.3 push olive past first NPC
        uint16_t ticks_to_walk_for_section2_3 = 950;
        size_t MAX_ATTEMPTS = 2;
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 5.95, ticks_to_walk_for_section2_3, 75, 10);
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
                    pbf_move_left_joystick(context, 128, 255, 200, 50);
                    pbf_wait(context, 5 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();                    
                    // ticks_to_walk_for_section2_3 = 500;                    
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE
                    throw e;
                }

            }
        }

        // section 2b. realign using fence corner
        direction.change_direction(env.program_info(), env.console, context,  4.5);
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        direction.change_direction(env.program_info(), env.console, context, 5.86);
        pbf_move_left_joystick(context, 128, 0, 300, 50);
        direction.change_direction(env.program_info(), env.console, context,  2.76);
        pbf_move_left_joystick(context, 128, 0, 50, 50);

        // section 3.1 push olive across the hump
        uint16_t ticks_to_walk_for_section3_1 = 350;
        uint16_t ticks_walked_section3_1 = 0;
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                ticks_walked_section3_1 = green.push_olive_forward(env.program_info(), env.console, context, 1.27, ticks_to_walk_for_section3_1, 125);
                break;
            }catch (OliveActionFailedException& e){
                // may have failed to push the olive past the hump. and walked past it
                if (i >= MAX_ATTEMPTS-1){
                    throw e;
                }

                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, 128, 255, 250, 50);
                    pbf_move_left_joystick(context, 128, 0, 50, 50);  // move forward slight in case the olive is undetectable since it's right in front of the character
                    pbf_wait(context, 5 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();                    
                    ticks_to_walk_for_section3_1 = 200;
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, 128, 255, 50, 50);
                    pbf_move_left_joystick(context, 128, 0, 150, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                    ticks_to_walk_for_section3_1 = 200;
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }
                
            }
        }

        
        // section 3.2 push olive across the hump and into fence
        uint16_t total_ticks_to_walk_for_section3 = 550;
        uint16_t ticks_to_walk_for_section3_2 = 0;
        if (ticks_walked_section3_1 < total_ticks_to_walk_for_section3){
            ticks_to_walk_for_section3_2 = (total_ticks_to_walk_for_section3 - ticks_walked_section3_1);
        }
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                green.push_olive_forward(env.program_info(), env.console, context, 1.27, ticks_to_walk_for_section3_2, 125);
                pbf_move_left_joystick(context, 128, 255, 100, 50);
                green.align_to_olive(env.program_info(), env.console, context, 1.27);
                green.walk_up_to_olive(env.program_info(), env.console, context, 1.27);
                break;
            }catch (OliveActionFailedException& e){
                // may have failed to push the olive past the hump. and walked past it
                if (i >= MAX_ATTEMPTS-1){
                    throw e;
                }
                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, 128, 255, 200, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();                    
                    ticks_to_walk_for_section3_2 += 200;
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, 128, 255, 50, 50);
                    pbf_move_left_joystick(context, 128, 0, 150, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }

            }
        }
        

        // section 3b. realign using fence.
        direction.change_direction(env.program_info(), env.console, context,  3.0);
        pbf_move_left_joystick(context, 128, 0, 75, 50);
        direction.change_direction(env.program_info(), env.console, context,  1.17);
        pbf_move_left_joystick(context, 128, 0, 100, 50);

        // section 4.1 last stretch. nudge the olive
        for (size_t i = 0; i < MAX_ATTEMPTS; i++){
            try{
                green.align_to_olive(env.program_info(), env.console, context, 6.0);
                green.walk_up_to_olive(env.program_info(), env.console, context, 6.0);                
                green.push_olive_forward(env.program_info(), env.console, context, 6.0, 50, 50);
                break;
            }catch (OliveActionFailedException& e){
                // may have failed to push the olive. and walked past it
                if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                    pbf_move_left_joystick(context, 128, 255, 200, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();                    
                }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                    // try moving back and then ramming forward
                    pbf_move_left_joystick(context, 128, 255, 50, 50);
                    pbf_move_left_joystick(context, 128, 0, 100, 50);
                    pbf_wait(context, 7 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }else{ // FAILED_PUSH_OLIVE_TOTAL_DISTANCE, 
                    throw e;
                }
            }
        }

        // section 4.2 past second NPC and into the finish line
        NoMinimapWatcher no_minimap(env.console, COLOR_RED, Milliseconds(5000));
        size_t MAX_ATTEMPTS_SECTION_4 = 3;
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                for (size_t i = 0; i < MAX_ATTEMPTS_SECTION_4; i++){
                    try{
                        green.push_olive_forward(env.program_info(), env.console, context, 6.0, 250);
                        green.push_olive_forward(env.program_info(), env.console, context, 5.8, 100);
                        green.push_olive_forward(env.program_info(), env.console, context, 6.0, 200);
                        green.push_olive_forward(env.program_info(), env.console, context, 6.1, 200);                        
                        break;
                    }catch (OliveActionFailedException& e){
                        // may have failed to push the olive. and walked past it
                        if (e.m_fail_reason == OliveFail::NO_OLIVE_DETECTED){
                            pbf_move_left_joystick(context, 128, 255, 200, 50);
                            pbf_wait(context, 7 * TICKS_PER_SECOND);
                            context.wait_for_all_requests();                            
                        }else if (e.m_fail_reason == OliveFail::FAILED_WALK_TO_OLIVE || e.m_fail_reason == OliveFail::OLIVE_STUCK){
                            // try moving back and then ramming forward
                            pbf_move_left_joystick(context, 128, 255, 50, 50);
                            pbf_move_left_joystick(context, 128, 0, 150, 50);
                            pbf_wait(context, 7 * TICKS_PER_SECOND);
                            context.wait_for_all_requests();
                            // then push angled towards the right
                            green.push_olive_forward(env.program_info(), env.console, context, 5.8, 100);                            
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
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);        

        enter_menu_from_overworld(env.program_info(), env.console, context, -1);

    });

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
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 128, 40});

    });

}





}
}
}
