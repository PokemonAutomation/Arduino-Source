/*  Material Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include <array>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV_MaterialFarmerTools.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


MaterialFarmerOptions::~MaterialFarmerOptions(){
//    ENABLE_SANDWICH.remove_listener(*this);
}

MaterialFarmerOptions::MaterialFarmerOptions(
    GroupOption::EnableMode enable_mode,
    OCR::LanguageOCROption* language_option,
    EventNotificationOption& notif_status_update_option,
    EventNotificationOption& notif_program_finish_option,
    EventNotificationOption& notif_error_recoverable_option,
    EventNotificationOption& notif_error_fatal_option
)
    : GroupOption(
        "Material Farmer",
        LockMode::UNLOCK_WHILE_RUNNING,
        enable_mode
    )
    , m_language_owner(language_option == nullptr
        ? new OCR::LanguageOCROption(
            "<b>Game Language:</b><br>Required to read sandwich ingredients.",
            IV_READER().languages(),
            LockMode::UNLOCK_WHILE_RUNNING,
            true
        )
        : nullptr
    )
    , RUN_TIME_IN_MINUTES(
        "<b>Run Duration:</b><br>Run the material farmer for this many minutes.",
        LockMode::UNLOCK_WHILE_RUNNING,
        32
    )
//    , SAVE_GAME_BEFORE_SANDWICH(
//        "<b>Save Game before each round:</b>",
//        LockMode::UNLOCK_WHILE_RUNNING,
//        true
//    )
//    , SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT("")
//    , NUM_SANDWICH_ROUNDS_STATIC_TEXT("")
    , LANGUAGE(language_option == nullptr ? *m_language_owner : *language_option)
    , SANDWICH_OPTIONS(
        "Make a Sandwich",
        &LANGUAGE,
        BaseRecipe::non_shiny,
        true,
        GroupOption::EnableMode::DEFAULT_DISABLED
    )
    , AUTO_HEAL_PERCENT(
        "<b>Auto-Heal %</b><br>Auto-heal if your HP drops below this percentage.",
        LockMode::UNLOCK_WHILE_RUNNING,
        75, 0, 100
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options: (developer only)</b></font>"
    )
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b><br>"
        "Set this on to save a Switch video everytime an error occurs. You can send the video to developers to help them debug later.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SKIP_WARP_TO_POKECENTER(
        "<b>Skip warping to closest PokeCenter:</b><br>"
        "This is for debugging the program without waiting for the initial warp.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
//    , ENABLE_SANDWICH(
//        "<b>Enable Sandwich making:</b><br>"
//        "This is for boosting spawn rates of specific Pokemon.",
//        LockMode::UNLOCK_WHILE_RUNNING,
//        false
//    )
    , TIME_PER_SANDWICH(
        "<b>Time per sandwich:</b><br>Number of minutes before resetting sandwich.",
        LockMode::UNLOCK_WHILE_RUNNING,
        30, 1, 30
    )
    , NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION(
        "<b>Number of forward moves per lets go iteration:</b><br>"
        "During Let's go autobattling sequence, the number of forward movements before resetting to Pokecenter.",
        LockMode::UNLOCK_WHILE_RUNNING,
        13
    )
    , NOTIFICATION_STATUS_UPDATE(notif_status_update_option)
    , NOTIFICATION_PROGRAM_FINISH(notif_program_finish_option)
    , NOTIFICATION_ERROR_RECOVERABLE(notif_error_recoverable_option)
    , NOTIFICATION_ERROR_FATAL(notif_error_fatal_option)
{
    PA_ADD_OPTION(RUN_TIME_IN_MINUTES);
//    PA_ADD_OPTION(ENABLE_SANDWICH);
//    PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH);
//    PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT);
//    PA_ADD_OPTION(NUM_SANDWICH_ROUNDS_STATIC_TEXT);
    if (m_language_owner){
        PA_ADD_OPTION(LANGUAGE);
    }
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(AUTO_HEAL_PERCENT);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
        PA_ADD_OPTION(SKIP_WARP_TO_POKECENTER);
        PA_ADD_OPTION(TIME_PER_SANDWICH);
        PA_ADD_OPTION(NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION);
    }

    MaterialFarmerOptions::on_config_value_changed(this);
//    ENABLE_SANDWICH.add_listener(*this);
}

void MaterialFarmerOptions::on_config_value_changed(void* object){

//    if (MaterialFarmerOptions::ENABLE_SANDWICH){
//        SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT.set_text(
//            "Saves the game before each sandwich.<br>"
//            "Recommended to leave on, as the sandwich maker will reset the game if it detects an error.<br>"
//        );
//        NUM_SANDWICH_ROUNDS_STATIC_TEXT.set_text(
//            "One sandwich per round.<br>"
//            "400-650 Happiny dust per sandwich, with Normal Encounter power level 2.<br>"
//            "(e.g. Chorizo x4, Banana x2, Mayo x3, Whipped Cream x1)<br>"
//        );
//        MaterialFarmerOptions::SANDWICH_OPTIONS.set_visibility(ConfigOptionState::ENABLED);
//    }else{
//        SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT.set_text("");
//        NUM_SANDWICH_ROUNDS_STATIC_TEXT.set_text("30 minutes per round.<br>");
//        MaterialFarmerOptions::SANDWICH_OPTIONS.set_visibility(ConfigOptionState::DISABLED);
//    }

}

// return new start time, so that minutes remaining is rounded up to a multiple of 32
WallClock new_start_time_after_reset(WallClock old_start_time, uint16_t run_time_in_minutes){
    auto farming_time_remaining = minutes_remaining(old_start_time, std::chrono::minutes(run_time_in_minutes));
    // round up the time to multiple of 32 (30 minutes per sandwich, plus 2 minutes for sandwich making)
    size_t desired_minutes_remaining = ((farming_time_remaining.count()/32)+1)*32;
    WallClock new_start_time = current_time() + std::chrono::minutes(desired_minutes_remaining) - std::chrono::minutes(run_time_in_minutes);
    return new_start_time;
}


void run_material_farmer(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    MaterialFarmerOptions& options,
    MaterialFarmerStats& stats
){
    OverworldSensors sensors(
        env.logger(), console, context
    );
    LetsGoEncounterBotTracker encounter_tracker(
        env, console,
        stats,
        sensors.lets_go_kill
    );
    WallClock start_time = current_time();
    WallClock last_sandwich_time = WallClock::min();
    LetsGoHpWatcher hp_watcher(COLOR_RED);

    // ensure we save before running the material farmer.
    // but no need to save if already saving prior to each sandwich
    if (!(options.SANDWICH_OPTIONS.enabled() && options.SANDWICH_OPTIONS.SAVE_GAME_BEFORE_SANDWICH)){
        save_game_from_overworld(env.program_info(), console, context);
    }    

    /* 
    - Use Let's Go along the path. Fly back to pokecenter when it reaches the end of the path.
    - Keeping repeating this for RUN_TIME_IN_MINUTES minutes
    */
    size_t consecutive_failures = 0;
    size_t max_consecutive_failures = 15;     
    while (true){
    try{
        while (true){
            // check time left on material farming
            auto farming_time_remaining = minutes_remaining(start_time, std::chrono::minutes(options.RUN_TIME_IN_MINUTES));
            console.log(
                "Time left in Material Farming: " + 
                std::to_string(farming_time_remaining.count()) + " min", 
                COLOR_PURPLE
            );
            if (farming_time_remaining <= std::chrono::minutes(0)){
                console.log("Time's up. Stop the Material farming program.", COLOR_RED);
                return;
            }

            // Check time left on sandwich
            if (options.SANDWICH_OPTIONS.enabled()){
                auto sandwich_time_remaining = minutes_remaining(last_sandwich_time, std::chrono::minutes(options.TIME_PER_SANDWICH));
                console.log(
                    "Time left on sandwich: " + 
                    std::to_string(sandwich_time_remaining.count()) + " min", 
                    COLOR_PURPLE
                );                   
                if (sandwich_time_remaining <= std::chrono::minutes(0)){
                    console.log("Sandwich not active. Make a sandwich.");
                    last_sandwich_time = make_sandwich_material_farm(env, console, context, options, stats);
                    console.overlay().add_log("Sandwich made.");

                    // Log time remaining in Material farming 
                    farming_time_remaining = minutes_remaining(start_time, std::chrono::minutes(options.RUN_TIME_IN_MINUTES));
                    console.log(
                        "Time left in Material Farming: " + 
                        std::to_string(farming_time_remaining.count()) + " min", 
                        COLOR_PURPLE
                    );
                    // Log time remaining on Sandwich
                    sandwich_time_remaining = minutes_remaining(last_sandwich_time, std::chrono::minutes(options.TIME_PER_SANDWICH));
                    console.log(
                        "Time left on sandwich: " + 
                        std::to_string(sandwich_time_remaining.count()) + " min", 
                        COLOR_PURPLE
                    );                  
                }else{
                    console.log("Sandwich is still active. Continue material farming.", COLOR_ORANGE);
                }
            }else{
                console.log("Sandwich making disabled. Skip sandwich checks.", COLOR_ORANGE);
            }

            // heal before starting Let's go
            console.log("Heal before starting Let's go", COLOR_PURPLE);
            console.log("Heal threshold: " + tostr_default(options.AUTO_HEAL_PERCENT), COLOR_PURPLE);
            check_hp(env, console, context, options, hp_watcher, stats);

            /*
            - Starts from pokemon center.
            - Flies to start position. Runs a Let's Go iteration. 
            - Then returns to pokemon center, regardless of whether 
            it completes the action or gets caught in a battle 
            */
            run_from_battles_and_back_to_pokecenter(env, console, context, stats,
                [&](ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
                    // Move to starting position for Let's Go hunting path
                    stream.log("Move to starting position for Let's Go hunting path.", COLOR_PURPLE);
                    move_to_start_position_for_letsgo1(env.program_info(), stream, context);

                    // run let's go while updating the HP watcher
                    stream.log("Starting Let's Go hunting path.", COLOR_PURPLE);
                    run_until<ProControllerContext>(
                        stream, context,
                        [&](ProControllerContext& context){
                            run_lets_go_iteration(stream, context, encounter_tracker, options.NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION);
                        },
                        {hp_watcher}
                    );
                } 
            ); 
            
            context.wait_for_all_requests();
        }
    }catch (OperationFailedException& e){
        stats.m_errors++;
        env.update_stats();
        e.send_notification(env, options.NOTIFICATION_ERROR_RECOVERABLE);

        // save screenshot after operation failed, 
        // dump_snapshot(console);

        if (options.SAVE_DEBUG_VIDEO){
            // Take a video to give more context for debugging
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            context.wait_for_all_requests();
        }

        consecutive_failures++;
        if (consecutive_failures >= max_consecutive_failures){
            throw e;
        }

        env.log("Reset game to handle recoverable error.");
        reset_game(env.program_info(), console, context);
        stats.m_game_resets++;
        env.update_stats();

        // update start time, so that minutes remaining is rounded up to a multiple of 32
        start_time = new_start_time_after_reset(start_time, options.RUN_TIME_IN_MINUTES);      
        // also update last sandwich time
        last_sandwich_time = WallClock::min();
    }
    }
}



void check_hp(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    MaterialFarmerOptions& options,
    LetsGoHpWatcher& hp_watcher,
    MaterialFarmerStats& stats
){
    double hp = hp_watcher.last_known_value() * 100;
    if (0 < hp){
        stream.log("Last Known HP: " + tostr_default(hp) + "%", COLOR_BLUE);
    }else{
        stream.log("Last Known HP: ?", COLOR_RED);
    }
    if (0 < hp && hp < options.AUTO_HEAL_PERCENT){
        auto_heal_from_menu_or_overworld(env.program_info(), stream, context, 0, true);
        stats.m_autoheals++;
        env.update_stats();
        send_program_status_notification(env, options.NOTIFICATION_STATUS_UPDATE);
    }
}



// make sandwich then go back to Pokecenter to reset position
// if gets caught up in a battle, try again.
WallClock make_sandwich_material_farm(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context, 
    MaterialFarmerOptions& options,
    MaterialFarmerStats& stats
){

    if (options.SANDWICH_OPTIONS.SAVE_GAME_BEFORE_SANDWICH){
        save_game_from_overworld(env.program_info(), stream, context);
    }

    WallClock last_sandwich_time = WallClock::min();
    while(last_sandwich_time == WallClock::min()){
        run_from_battles_and_back_to_pokecenter(env, stream, context, stats,
            [&](ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
                // Orient camera to look at same direction as player character
                // - This is needed because when save-load the game, 
                // the camera angle is different than when just flying to pokecenter
                pbf_press_button(context, BUTTON_L, 50, 40);

                // move up towards pokecenter counter        
                pbf_move_left_joystick(context, 128, 255, 180, 10);
                // Orient camera to look at same direction as player character
                pbf_press_button(context, BUTTON_L, 50, 40);
                // look left
                pbf_move_right_joystick(context, 0, 128, 120, 0);
                // move toward clearing besides the pokecenter
                pbf_move_left_joystick(context, 128, 0, 300, 0);

                // make sandwich
                picnic_from_overworld(env.program_info(), stream, context);
                pbf_move_left_joystick(context, 128, 0, 100, 40);  // walk forward to picnic table
                enter_sandwich_recipe_list(env.program_info(), stream, context);
                make_sandwich_option(env, stream, context, options.SANDWICH_OPTIONS);
                last_sandwich_time = current_time();
                leave_picnic(env.program_info(), stream, context);

                stats.m_sandwiches++;
                env.update_stats();

            }
        );
    }
    
    return last_sandwich_time;
}

// given the start time, and duration in minutes, return the number of remaining minutes
// WARNING: this function may silently overflow if start_time is near WallClock::min() or WallClock::max()
std::chrono::minutes minutes_remaining(WallClock start_time, std::chrono::minutes minutes_duration){
    if (start_time == WallClock::min()){
        return std::chrono::minutes(0);
    }else{
        auto elapsed_time = std::chrono::duration_cast<std::chrono::minutes>(current_time() - start_time);
        return minutes_duration - elapsed_time;
    }
}

// from the North Province (Area 3) pokecenter, move to start position for Happiny dust farming
void move_to_start_position_for_letsgo0(    
    VideoStream& stream,
    ProControllerContext& context
){
    // Orient camera to look at same direction as player character
    // - This is needed because when save-load the game, 
    // the camera angle is different than when just flying to pokecenter
    pbf_press_button(context, BUTTON_L, 50, 40);

    // move up towards pokecenter counter        
    pbf_move_left_joystick(context, 128, 255, 180, 10);
    // Orient camera to look at same direction as player character
    pbf_press_button(context, BUTTON_L, 50, 40);
    // look left
    pbf_move_right_joystick(context, 0, 128, 120, 10);
    // move toward clearing besides the pokecenter
    pbf_move_left_joystick(context, 128, 0, 300, 10);

    // look right, towards the start position
    pbf_move_right_joystick(context, 255, 128, 120, 10);
    pbf_move_left_joystick(context, 128, 0, 10, 10);

    // get on ride
    pbf_press_button(context, BUTTON_PLUS, 50, 50);

    // Jump
    pbf_press_button(context, BUTTON_B, 125, 100);

    // Fly 
    pbf_press_button(context, BUTTON_B, 50, 10); //  Double up this press 
    pbf_press_button(context, BUTTON_B, 50, 10);     //  in case one is dropped.
    pbf_press_button(context, BUTTON_LCLICK, 50, 0);
    // you automatically move forward without pressing any buttons. so just wait
    pbf_wait(context, 1400);

    // Glide forward
    // pbf_move_left_joystick(context, 128, 0, 2500, 10);

    // arrived at start position. stop flying
    pbf_press_button(context, BUTTON_B, 50, 400);
    // get off ride
    pbf_press_button(context, BUTTON_PLUS, 50, 50);

    // look right
    pbf_move_right_joystick(context, 255, 128, 30, 10);
    pbf_move_left_joystick(context, 128, 0, 50, 10);

    stream.log("Arrived at Let's go start position", COLOR_PURPLE);
    

}

// from the North Province (Area 3) pokecenter, move to start position for Happiny dust farming
void move_to_start_position_for_letsgo1(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context
){
    // Orient camera to look at same direction as player character
    // - This is needed because when save-load the game, 
    // the camera angle is different than when just flying to pokecenter
    pbf_press_button(context, BUTTON_L, 50, 40);

    // move up towards pokecenter counter        
    pbf_move_left_joystick(context, 128, 255, 180, 10);
    // Orient camera to look at same direction as player character
    pbf_press_button(context, BUTTON_L, 50, 40);
    // look left
    pbf_move_right_joystick(context, 0, 128, 120, 10);
    // move toward clearing besides the pokecenter
    pbf_move_left_joystick(context, 128, 0, 300, 10);

    // look right, towards the start position
    DirectionDetector direction;
    direction.change_direction(info, stream, context, 5.76);
    // pbf_move_right_joystick(context, 255, 128, 130, 10);
    pbf_move_left_joystick(context, 128, 0, 10, 10);

    // get on ride
    pbf_press_button(context, BUTTON_PLUS, 50, 50);

    // Jump
    pbf_press_button(context, BUTTON_B, 125, 30);

    // Fly 
    pbf_press_button(context, BUTTON_B, 50, 10);
    pbf_press_button(context, BUTTON_B, 50, 50); // Double click in case of drop
    pbf_press_button(context, BUTTON_LCLICK, 50, 0);

    // you automatically move forward  when flying without pressing any buttons. 
    // so, just wait.
    pbf_wait(context, 2200);

    // arrived at start position. stop flying
    pbf_press_button(context, BUTTON_B, 50, 400);
    // get off ride
    pbf_press_button(context, BUTTON_PLUS, 50, 50);

    // extra B presses to ensure we stop flying, in case the previous B press
    // was dropped. This way, you eventually reset back to Pokecenter, instead
    // of flying until an exception is thrown.
    pbf_press_button(context, BUTTON_B, 50, 10);
    pbf_press_button(context, BUTTON_B, 50, 10);

    // look right
    // pbf_move_right_joystick(context, 255, 128, 20, 10);
    direction.change_direction(info, stream, context, 5.3);

    // move forward slightly
    pbf_move_left_joystick(context, 128, 0, 50, 10);

    stream.log("Arrived at Let's go start position", COLOR_PURPLE);
}


// wait, then move forward quickly
void lets_go_movement0(ProControllerContext& context){
    pbf_wait(context, 500);
    pbf_move_left_joystick(context, 128, 0, 200, 10);
}

// wait, then move forward quickly, then wait some more.
void lets_go_movement1(ProControllerContext& context){
    pbf_wait(context, 500);
    pbf_move_left_joystick(context, 128, 0, 100, 10);
    pbf_wait(context, 100);
}


/*
- One iteration of the hunt: 
- start at North Province (Area 3) pokecenter, go out and use Let's Go to battle
- move forward and send out lead pokemon to autobattle. When it runs out of pokemon to battle, 
move forward again to repeat the cycle. It does this as many times as per num_forward_moves_per_lets_go_iteration.
 */
void run_lets_go_iteration(
    VideoStream& stream,
    ProControllerContext& context,
    LetsGoEncounterBotTracker& encounter_tracker,
    int num_forward_moves_per_lets_go_iteration
){
    // - Orient camera to look at same direction as player character
    // - This is needed because when save-load the game, the camera points
    // in the same direction as the player.
    // - But when warping to pokecenter, the camera is facing the player.
    pbf_press_button(context, BUTTON_L, 50, 40);

    // zoom out camera
    pbf_move_right_joystick(context, 128, 255, 45, 10);

    const bool throw_ball_if_bubble = false;
    const int total_iterations = num_forward_moves_per_lets_go_iteration;

    context.wait_for_all_requests();
    for(int i = 0; i < total_iterations; i++){
        use_lets_go_to_clear_in_front(stream, context, encounter_tracker, throw_ball_if_bubble, [&](ProControllerContext& context){
            // Do the following movement while the Let's Go pokemon clearing wild pokemon.
            stream.log("Move-forward iteration number: " + std::to_string(i + 1) + "/" + std::to_string(total_iterations), COLOR_PURPLE);

            lets_go_movement1(context);
        });
    }

}

/* 
- This function wraps around an action (e.g. leave the PokeCenter to make a sandwich) so that
we can handle pokemon wild encounters when executing the action.
  - note that returning to the PokeCenter is also wrapped, since it's possible to get caught
  in a wild encounter when trying to return to the PokeCenter.
- Whenever a battle happens, we run away and after the battle ends, move back to the PokeCenter
- When `action` ends, go back to the PokeCenter.
- i.e. return to Pokecenter regardless of whether the action succeeds or not.
- NOTE: This works differently than `handle_battles_and_back_to_pokecenter` from the Scatterbug program,
where the action will be attempted infinite times if you keep failing. In contrast, this function
only gives you one attempt before returning to the Pokecenter
*/
void run_from_battles_and_back_to_pokecenter(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    MaterialFarmerStats& stats,
    std::function<
        void(ProgramEnvironment& env,
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
){
    bool attempted_action = false;
    // a flag for the case that the action has finished but not yet returned to pokecenter
    bool returned_to_pokecenter = false;
    while(returned_to_pokecenter == false){
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
                if (!attempted_action){ // We still need to carry out `action`
                    attempted_action = true;
                    context.wait_for_all_requests();
                    action(env, stream, context);
                    context.wait_for_all_requests();                    
                }

                // we have already attempted the action,
                // so reset to the Pokecenter
                stream.log("Go back to PokeCenter.");
                reset_to_pokecenter(env.program_info(), stream, context);
                returned_to_pokecenter = true;
            },
            {battle_menu}
        );
        if (ret == 0){ // battle detected
            stats.m_encounters++;
            env.update_stats();
            stream.log("Detected battle. Now running away.", COLOR_PURPLE);
            stream.overlay().add_log("Detected battle. Now running away.");
            try{
                run_from_battle(stream, context);
            }catch (OperationFailedException& e){
                throw FatalProgramException(std::move(e));
            }
        }
    } 
}




void move_from_material_farming_to_item_printer(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Start moving from material farming to item printer.");
    fly_from_paldea_to_blueberry_entrance(info, stream, context);
    move_from_blueberry_entrance_to_league_club(info, stream, context);
    move_from_league_club_entrance_to_item_printer(info, stream, context);
}

void fly_from_paldea_to_blueberry_entrance(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    int numAttempts = 0;
    const int maxAttempts = 11;
    bool isFlySuccessful = false;

    // in order to land on the target fly point semi-consistently, 
    // the push magnitude can range from 69 to 85 (range of 16). 
    // On each failure, try increasing/decreasing the push by 1/4 of the max range,
    // then 1/2 of the range, then the full range, then back to re-attempts with no adjustment
    const std::array<int, maxAttempts + 1> adjustment_table =  {0, 0, 0, 4, -4, 8, -8, 16, -16, 0, 0, 0};
    while (!isFlySuccessful && numAttempts < maxAttempts){
        // close all menus
        pbf_mash_button(context, BUTTON_B, 100);

        numAttempts++;

        open_map_from_overworld(info, stream, context);

        // change from Paldea map to Blueberry map
        pbf_press_button(context, BUTTON_L, 50, 300);

        // move cursor to bottom right corner
        pbf_move_left_joystick(context, 255, 255, TICKS_PER_SECOND*5, 50);

        // move cursor to Blueberry academy fast travel point (up-left)
        // try different magnitudes of cursor push with each failure.
        int push_magnitude = 105 + adjustment_table[numAttempts];
        pbf_move_left_joystick(context, 64, 64, (uint16_t)push_magnitude, 50);

        // press A to fly to Blueberry academy
        isFlySuccessful = fly_to_overworld_from_map(info, stream, context, true);
        if (!isFlySuccessful){
            stream.log("Failed to fly to Blueberry academy.");
            // dump_snapshot(stream);
        }
    }

    if (!isFlySuccessful){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to fly to Blueberry academy, five times in a row.",
            stream
        );
    }
}

void move_from_blueberry_entrance_to_league_club(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

    int numAttempts = 0;
    int maxAttempts = 5;
    bool isSuccessful = false;

    while (!isSuccessful && numAttempts < maxAttempts){
        if (numAttempts > 0){ // failed at least once
            pbf_mash_button(context, BUTTON_B, 100);
            open_map_from_overworld(info, stream, context);
            fly_to_overworld_from_map(info, stream, context, false);
        }

        numAttempts++;

        // move toward entrance gates
        pbf_move_left_joystick(context, 190, 0, 200, 50);

        context.wait_for_all_requests();

        // Wait for detection of Blueberry navigation menu
        ImageFloatBox select_entrance_box(0.031, 0.193, 0.047, 0.078);
        GradientArrowWatcher select_entrance(COLOR_RED, GradientArrowType::RIGHT, select_entrance_box);
        int ret = wait_until(stream, context, Milliseconds(5000), { select_entrance });
        if (ret == 0){
            stream.log("Blueberry navigation menu detected.");
        }else{
            stream.log("Failed to detect Blueberry navigation menu.");
            continue;
        }

        // Move selector to League club room
        pbf_press_dpad(context, DPAD_UP, 20, 50);

        // Confirm to League club room is selected
        ImageFloatBox select_league_club_box(0.038, 0.785, 0.043, 0.081);
        GradientArrowWatcher select_league_club(COLOR_RED, GradientArrowType::RIGHT, select_league_club_box, Milliseconds(1000));
        ret = wait_until(stream, context, Milliseconds(5000), { select_league_club });
        if (ret == 0){
            stream.log("League club room selected.");
        }else{
            stream.log("Failed to select League club room in navigation menu.");
            continue;            
        }
        // press A
        pbf_mash_button(context, BUTTON_A, 100);

        // check for overworld
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        ret = wait_until(stream, context, Milliseconds(10000), { overworld });
        if (ret == 0){
            stream.log("Entered League club room.");
        }else{
            stream.log("Failed to enter League club room from menu selection.");
            continue;            
        }

        isSuccessful = true;
    }

    if (!isSuccessful){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter League club room, five times in a row.",
            stream
        );
    }

}

void move_from_league_club_entrance_to_item_printer(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();

    // move forwards towards table next to item printer
    pbf_move_left_joystick(context, 120, 0, 200, 50);

    // look left towards item printer
    pbf_move_left_joystick(context, 0, 128, 10, 50);
}

void move_from_item_printer_to_material_farming(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Start moving from item printer to material farming.");
    move_from_item_printer_to_blueberry_entrance(info, stream, context);
    fly_from_blueberry_to_north_province_3(info, stream, context);
}

// assumes you start in the position in front of the item printer, as if you finished using it.
void move_from_item_printer_to_blueberry_entrance(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();

    // look left towards door
    pbf_move_left_joystick(context, 0, 128, 10, 50);

    // re-orient camera to look same direction as player
    pbf_press_button(context, BUTTON_L, 50, 50);

    // move forward towards door
    pbf_move_left_joystick(context, 128, 0, 700, 50);

    context.wait_for_all_requests();

    stream.log("Wait for detection of Blueberry navigation menu.");

    // Wait for detection of Blueberry navigation menu
    ImageFloatBox select_entrance_box(0.031, 0.193, 0.047, 0.078);
    GradientArrowWatcher select_entrance(COLOR_RED, GradientArrowType::RIGHT, select_entrance_box);
    int ret = wait_until(stream, context, Milliseconds(5000), { select_entrance }, Milliseconds(1000));
    if (ret == 0){
        stream.log("Blueberry navigation menu detected.");
    }else{
        stream.log("Failed to detect Blueberry navigation menu.");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to find the exit from the League room.",
            stream
        );
    }

    // press A
    pbf_mash_button(context, BUTTON_A, 100);    

    // check for overworld
    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
    ret = wait_until(stream, context, std::chrono::seconds(30), { overworld });
    if (ret == 0){
        stream.log("Overworld detected");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to detect overworld.",
            stream
        );      
    }
}


void fly_from_blueberry_to_north_province_3(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    int numAttempts = 0;
    const int maxAttempts = 11;
    bool isFlySuccessful = false;

    // in order to land on the target fly point, the push magnitude can range from
    // 156 to 172 (range of 16). 
    // On each failure, try increasing/decreasing the push by 1/4 of the max range,
    // then 1/2 of the range, then the full range, then back to re-attempts with no adjustment
    const std::array<int, maxAttempts + 1> adjustment_table =  {0, 0, 0, 4, -4, 8, -8, 16, -16, 0, 0, 0};
    while (!isFlySuccessful && numAttempts < maxAttempts){
        numAttempts++;

        // close all menus
        pbf_mash_button(context, BUTTON_B, 100);

        open_map_from_overworld(info, stream, context);

        // change from Blueberry map to Paldea map
        pbf_press_button(context, BUTTON_R, 50, 300);

        // zoom out
        pbf_press_button(context, BUTTON_ZL, 25, 200);

        // move cursor up-left
        // try different magnitudes of cursor push with each failure.
        int push_magnitude = 168 + adjustment_table[numAttempts];
        pbf_move_left_joystick(context, 112, 0, (uint16_t)push_magnitude, 50);

        // press A to fly to North province area 3
        isFlySuccessful = fly_to_overworld_from_map(info, stream, context, true);

        if (!isFlySuccessful){
            stream.log("Failed to fly to North province area 3.");
            // dump_snapshot(console);
        }
    }

    if (!isFlySuccessful){

        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to fly to North province area 3, ten times in a row.",
            stream
        );

    }
}



}
}
}
