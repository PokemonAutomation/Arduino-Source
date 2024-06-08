/*  Material Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */
#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"

#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_ShinyHunt-Scatterbug.h"
#include "NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


MaterialFarmerOptions::~MaterialFarmerOptions(){
    ENABLE_SANDWICH.remove_listener(*this);
}

MaterialFarmerOptions::MaterialFarmerOptions(
    OCR::LanguageOCROption* language_option,
    GoHomeWhenDoneOption* go_home_when_done_option,
    EventNotificationOption& notif_status_update_option,
    EventNotificationOption& notif_program_finish_option,
    EventNotificationOption& notif_error_recoverable_option,
    EventNotificationOption& notif_error_fatal_option
)
    : GroupOption(
        "Material Farmer",
        LockMode::UNLOCK_WHILE_RUNNING,
        false, true
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
    , m_go_home_when_done_owner(go_home_when_done_option == nullptr 
        ? new GoHomeWhenDoneOption(true) 
        : nullptr
    )
    , SAVE_GAME_BEFORE_SANDWICH(
        "<b>Save Game before each round:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT("")
    , NUM_SANDWICH_ROUNDS(
        "<b>Number of rounds to run:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        3
    )
    , NUM_SANDWICH_ROUNDS_STATIC_TEXT("")
    , LANGUAGE(language_option == nullptr ? *m_language_owner : *language_option)
    , SANDWICH_OPTIONS(LANGUAGE)
    , GO_HOME_WHEN_DONE(go_home_when_done_option == nullptr ? *m_go_home_when_done_owner : *go_home_when_done_option)
    , AUTO_HEAL_PERCENT(
        "<b>Auto-Heal %</b><br>Auto-heal if your HP drops below this percentage.",
        LockMode::UNLOCK_WHILE_RUNNING,
        75, 0, 100
    )
    , SAVE_DEBUG_VIDEO(
        "<b>DEV MODE: Save debug videos to Switch:</b><br>"
        "Set this on to save a Switch video everytime an error occurs. You can send the video to developers to help them debug later.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SKIP_WARP_TO_POKECENTER(
        "<b>DEV MODE: Skip warping to closest PokeCenter:</b><br>"
        "This is for debugging the program without waiting for the initial warp.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , ENABLE_SANDWICH(
        "<b>Enable Sandwich making:</b><br>"
        "This is for boosting spawn rates of specific Pokemon.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , TIME_PER_SANDWICH(
        "<b>DEV MODE: Time per sandwich:</b><br>Number of minutes before resetting sandwich.",
        LockMode::UNLOCK_WHILE_RUNNING,
        30, 1, 30
    )
    , NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION(
        "<b>DEV MODE: Number of forward moves per lets go iteration:</b><br>"
        "During Let's go autobattling sequence, the number of forward movements before resetting to Pokecenter.",
        LockMode::UNLOCK_WHILE_RUNNING,
        13
    )
    , NOTIFICATION_STATUS_UPDATE(notif_status_update_option)
    , NOTIFICATION_PROGRAM_FINISH(notif_program_finish_option)
    , NOTIFICATION_ERROR_RECOVERABLE(notif_error_recoverable_option)
    , NOTIFICATION_ERROR_FATAL(notif_error_fatal_option)
{
    PA_ADD_OPTION(ENABLE_SANDWICH);
    PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH);
    PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT);
    PA_ADD_OPTION(NUM_SANDWICH_ROUNDS);
    PA_ADD_OPTION(NUM_SANDWICH_ROUNDS_STATIC_TEXT);
    if (m_language_owner){
        PA_ADD_OPTION(LANGUAGE);
    }
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    if(m_go_home_when_done_owner){
        PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    }
    PA_ADD_OPTION(AUTO_HEAL_PERCENT);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
        PA_ADD_OPTION(SKIP_WARP_TO_POKECENTER);
        PA_ADD_OPTION(TIME_PER_SANDWICH);
        PA_ADD_OPTION(NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION);
    }

    MaterialFarmerOptions::value_changed();
    ENABLE_SANDWICH.add_listener(*this);
}

void MaterialFarmerOptions::value_changed(){

    if (MaterialFarmerOptions::ENABLE_SANDWICH){
        SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT.set_text(
            "Saves the game before each sandwich.<br>"
            "Recommended to leave on, as the sandwich maker will reset the game if it detects an error.<br>"
        );
        NUM_SANDWICH_ROUNDS_STATIC_TEXT.set_text(
            "One sandwich per round.<br>"
            "400-650 Happiny dust per sandwich, with Normal Encounter power level 2.<br>"
            "(e.g. Chorizo x4, Banana x2, Mayo x3, Whipped Cream x1)<br>"
        );
        MaterialFarmerOptions::SANDWICH_OPTIONS.set_visibility(ConfigOptionState::ENABLED);
    }else{
        SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT.set_text("");
        NUM_SANDWICH_ROUNDS_STATIC_TEXT.set_text("30 minutes per round.<br>");
        MaterialFarmerOptions::SANDWICH_OPTIONS.set_visibility(ConfigOptionState::DISABLED);
    }

}




void run_material_farmer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    MaterialFarmerOptions& options, MaterialFarmerStats& stats
){
    
    LetsGoEncounterBotTracker encounter_tracker(
        env, env.console, context,
        stats,
        options.LANGUAGE
    );


    size_t consecutive_failures;
    size_t max_consecutive_failures = 10;    
    bool done_sandwich_iteration;
    for (uint16_t i = 0; i < options.NUM_SANDWICH_ROUNDS; i++){
        consecutive_failures = 0;
        done_sandwich_iteration = false;
        while (!done_sandwich_iteration){
            try{
                run_one_sandwich_iteration(env, context, encounter_tracker, options);
                done_sandwich_iteration = true;
            }catch(OperationFailedException& e){
                stats.m_errors++;
                env.update_stats();
                e.send_notification(env, options.NOTIFICATION_ERROR_RECOVERABLE);

                // save screenshot after operation failed, 
                dump_snapshot(env.console);

                if (options.SAVE_DEBUG_VIDEO){
                    // Take a video to give more context for debugging
                    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }

                consecutive_failures++;
                if (consecutive_failures >= max_consecutive_failures){
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, env.console,
                        "Failed 10 times in a row.",
                        true
                    );
                }

                env.log("Reset game to handle recoverable error");
                reset_game(env.program_info(), env.console, context);
                ++stats.m_game_resets;
                env.update_stats();
            }catch(ProgramFinishedException&){
                options.GO_HOME_WHEN_DONE.run_end_of_program(context);
                throw;
            }
        }
    }
}



// start at North Province (Area 3) Pokecenter, make a sandwich, then use let's go repeatedly until 30 min passes.
void run_one_sandwich_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker, MaterialFarmerOptions& options)
{

    MaterialFarmerStats& stats = env.current_stats<MaterialFarmerStats>();

    WallClock last_sandwich_time = WallClock::min();

    if (options.SAVE_GAME_BEFORE_SANDWICH){
        save_game_from_overworld(env.program_info(), env.console, context);
    }
    // make sandwich then go back to Pokecenter to reset position
    if (options.ENABLE_SANDWICH){
        run_from_battles_and_back_to_pokecenter(env, context, 
            [&](SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
                picnic_from_overworld(env.program_info(), env.console, context);
                pbf_move_left_joystick(context, 128, 0, 30, 40);
                enter_sandwich_recipe_list(env.program_info(), env.console, context);
                make_sandwich_option(env, env.console, context, options.SANDWICH_OPTIONS);
                last_sandwich_time = current_time();
                leave_picnic(env.program_info(), env.console, context);

                stats.m_sandwiches++;
                env.update_stats();

            }
        );
    }
    else {
        last_sandwich_time = current_time();
    }


    LetsGoHpWatcher hp_watcher(COLOR_RED);

    /* 
    - Use Let's Go along the path. Fly back to pokecenter when it reaches the end of the path.
    - Keeping repeating this until the sandwich expires.
     */
    while (true){
        int time_per_sandwich = options.TIME_PER_SANDWICH;
        std::chrono::minutes minutes_per_sandwich = std::chrono::minutes(time_per_sandwich);
        auto sandwich_time_left =  minutes_per_sandwich + std::chrono::duration_cast<std::chrono::minutes>(last_sandwich_time - current_time());
        env.console.log(
            "Time left on sandwich: " + 
            std::to_string(sandwich_time_left.count()) + " min", 
            COLOR_PURPLE);

        if (is_sandwich_expired(last_sandwich_time, minutes_per_sandwich)){
            env.log("Sandwich expired. Start another sandwich round.");
            env.console.overlay().add_log("Sandwich expired.");
            break;
        }

        // heal before starting Let's go
        env.console.log("Heal before starting Let's go", COLOR_PURPLE);
        env.console.log("Heal threshold: " + tostr_default(options.AUTO_HEAL_PERCENT), COLOR_PURPLE);
        double hp = hp_watcher.last_known_value() * 100;
        if (0 < hp){
            env.console.log("Last Known HP: " + tostr_default(hp) + "%", COLOR_BLUE);
        }else{
            env.console.log("Last Known HP: ?", COLOR_RED);
        }
        if (0 < hp && hp < options.AUTO_HEAL_PERCENT){
            auto_heal_from_menu_or_overworld(env.program_info(), env.console, context, 0, true);
            stats.m_autoheals++;
            env.update_stats();
            send_program_status_notification(env, options.NOTIFICATION_STATUS_UPDATE);
        }

        /*
        - Starts from pokemon center.
        - Flies to start position. Runs a Let's Go iteration. 
        - Then returns to pokemon center 
        */
        env.console.log("Starting Let's Go hunting path", COLOR_PURPLE);
        int num_forward_moves_per_lets_go_iteration = options.NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION;
        run_from_battles_and_back_to_pokecenter(env, context, 
            [&](
                SingleSwitchProgramEnvironment& env, BotBaseContext& context
            ){
                run_until(
                    env.console, context,
                    [&](BotBaseContext& context){

                        /*                         
                        - run_from_battles_and_back_to_pokecenter will keep looping `action` 
                        (i.e. this lambda function) until it succeeeds
                        - Do a sandwich time check here to break out of the loop, in the case where
                        you are very unlucky and can't finish a Let's Go iteration due to getting caught
                        up in battles.
                        */
                        if (is_sandwich_expired(last_sandwich_time, minutes_per_sandwich)){
                            env.log("Sandwich expired. Return to Pokecenter.");
                            return;
                        }                        
                        move_to_start_position_for_letsgo1(env, context);
                        run_lets_go_iteration(env, context, encounter_tracker, num_forward_moves_per_lets_go_iteration);
                    },
                    {hp_watcher}
                );
            } 
        ); 
        
        context.wait_for_all_requests();
    }


}



bool is_sandwich_expired(WallClock last_sandwich_time, std::chrono::minutes minutes_per_sandwich){
    return (last_sandwich_time + minutes_per_sandwich) < current_time();
}

// from the North Province (Area 3) pokecenter, move to start position for Happiny dust farming
void move_to_start_position_for_letsgo0(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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

    env.console.log("Arrived at Let's go start position", COLOR_PURPLE);
    

}

// from the North Province (Area 3) pokecenter, move to start position for Happiny dust farming
void move_to_start_position_for_letsgo1(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
    pbf_move_right_joystick(context, 255, 128, 130, 10);
    pbf_move_left_joystick(context, 128, 0, 10, 10);

    // get on ride
    pbf_press_button(context, BUTTON_PLUS, 50, 50);

    // Jump
    pbf_press_button(context, BUTTON_B, 125, 30);

    // Fly 
    pbf_press_button(context, BUTTON_B, 50, 10); 
    pbf_press_button(context, BUTTON_B, 50, 10); // Double click in case of drop
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
    pbf_move_right_joystick(context, 255, 128, 20, 10);

    // move forward slightly
    pbf_move_left_joystick(context, 128, 0, 50, 10);

    env.console.log("Arrived at Let's go start position", COLOR_PURPLE);
}


// wait, then move forward quickly
void lets_go_movement0(BotBaseContext& context){
    pbf_wait(context, 500);
    pbf_move_left_joystick(context, 128, 0, 200, 10);
}

// wait, then move forward quickly, then wait some more.
void lets_go_movement1(BotBaseContext& context){
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
void run_lets_go_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker, int num_forward_moves_per_lets_go_iteration)
{
    auto& console = env.console;
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
        use_lets_go_to_clear_in_front(console, context, encounter_tracker, throw_ball_if_bubble, [&](BotBaseContext& context){
            // Do the following movement while the Let's Go pokemon clearing wild pokemon.
            env.console.log("Move-forward iteration number: " + std::to_string(i + 1) + "/" + std::to_string(total_iterations), COLOR_PURPLE);

            lets_go_movement1(context);
        });
    }

}

/* 
- This function wraps around an action (e.g. go out of PokeCenter to make a sandwich) so that
we can handle pokemon wild encounters when executing the action.
- Whenever a battle happens, we run away.
- After battle ends, move back to PokeCenter to start the `action` again.
- `action` must be an action starting at the PokeCenter 
*/
void run_from_battles_and_back_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    std::function<void(SingleSwitchProgramEnvironment& env, BotBaseContext& context)>&& action)
{
    MaterialFarmerStats& stats = env.current_stats<MaterialFarmerStats>();
    bool action_finished = false;
    bool first_iteration = true;
    // a flag for the case that the action has finished but not yet returned to pokecenter
    bool returned_to_pokecenter = false;
    while(action_finished == false || returned_to_pokecenter == false){
        // env.console.overlay().add_log("Calculate what to do next");
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                if (action_finished){
                    // `action` is already finished. Now we just try to get back to pokecenter:
                    reset_to_pokecenter(env.program_info(), env.console, context);
                    context.wait_for_all_requests();
                    returned_to_pokecenter = true;
                    return;
                }
                // We still need to carry out `action`
                if (first_iteration){
                    first_iteration = false;
                }
                else{
                    // This is at least the second iteration in the while-loop.
                    // So a previous round of action failed.
                    // We need to first re-initialize our position to the PokeCenter
                    // Use map to fly back to the pokecenter
                    reset_to_pokecenter(env.program_info(), env.console, context);
                }
                context.wait_for_all_requests();
                action(env, context);
                context.wait_for_all_requests();
                action_finished = true;
            },
            {battle_menu}
        );
        if (ret == 0){
            stats.m_encounters++;
            env.update_stats();
            env.console.log("Detected battle. Now running away.", COLOR_PURPLE);
            env.console.overlay().add_log("Detected battle. Now running away.");
            try{
                run_from_battle(env.program_info(), env.console, context);
            }catch (OperationFailedException& e){
                throw FatalProgramException(std::move(e));
            }
        }
        // Back on the overworld.
    } // end while(action_finished == false || returned_to_pokecenter == false)
}




void move_from_material_farming_to_item_printer(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Start moving from material farming to item printer.");
    fly_from_paldea_to_blueberry_entrance(env, context);
    move_from_blueberry_entrance_to_league_club(env, context);
    move_from_league_club_entrance_to_item_printer(env, context);
}

void fly_from_paldea_to_blueberry_entrance(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    int numAttempts = 0;
    int maxAttempts = 5;
    bool isFlySuccessful = false;

    while (!isFlySuccessful && numAttempts < maxAttempts){
        // close all menus
        pbf_mash_button(context, BUTTON_B, 100);

        numAttempts++;

        open_map_from_overworld(env.program_info(), env.console, context);

        // change from Paldea map to Blueberry map
        pbf_press_button(context, BUTTON_L, 50, 300);

        // move cursor to bottom right corner
        pbf_move_left_joystick(context, 255, 255, TICKS_PER_SECOND*5, 50);

        // move cursor to Blueberry academy fast travel point (up-left)
        pbf_move_left_joystick(context, 0, 0, 76, 50);

        // press A to fly to Blueberry academy
        isFlySuccessful = fly_to_overworld_from_map(env.program_info(), env.console, context, true);
        if (!isFlySuccessful){
            env.log("Failed to fly to Blueberry academy.");
        }
    }

    if (!isFlySuccessful){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Failed to fly to Blueberry academy, five times in a row.",
            true
        );
    }
}

void move_from_blueberry_entrance_to_league_club(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    int numAttempts = 0;
    int maxAttempts = 5;
    bool isSuccessful = false;

    while (!isSuccessful && numAttempts < maxAttempts){
        if (numAttempts > 0){ // failed at least once
            pbf_mash_button(context, BUTTON_B, 100);
            open_map_from_overworld(env.program_info(), env.console, context);
            fly_to_overworld_from_map(env.program_info(), env.console, context, false);
        }

        numAttempts++;

        // move toward entrance gates
        pbf_move_left_joystick(context, 190, 0, 200, 50);

        context.wait_for_all_requests();

        // Wait for detection of Blueberry navigation menu
        ImageFloatBox select_entrance_box(0.031, 0.193, 0.047, 0.078);
        GradientArrowWatcher select_entrance(COLOR_RED, GradientArrowType::RIGHT, select_entrance_box);
        int ret = wait_until(env.console, context, Milliseconds(5000), { select_entrance });
        if (ret == 0){
            env.log("Blueberry navigation menu detected.");
        }else{
            env.console.log("Failed to detect Blueberry navigation menu.");
            continue;
        }

        // Move selector to League club room
        pbf_press_dpad(context, DPAD_UP, 20, 50);

        // Confirm to League club room is selected
        ImageFloatBox select_league_club_box(0.038, 0.785, 0.043, 0.081);
        GradientArrowWatcher select_league_club(COLOR_RED, GradientArrowType::RIGHT, select_league_club_box, Milliseconds(1000));
        ret = wait_until(env.console, context, Milliseconds(5000), { select_league_club });
        if (ret == 0){
            env.log("League club room selected.");
        }else{
            env.console.log("Failed to select League club room in navigation menu.");
            continue;            
        }
        // press A
        pbf_mash_button(context, BUTTON_A, 100);

        // check for overworld
        OverworldWatcher overworld(COLOR_CYAN);  
        ret = wait_until(env.console, context, Milliseconds(10000), { overworld });
        if (ret == 0){
            env.log("Entered League club room.");
        }else{
            env.console.log("Failed to enter League club room from menu selection.");
            continue;            
        }

        isSuccessful = true;
    }

    if (!isSuccessful){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Failed to enter League club room, five times in a row.",
            true
        );
    }

}

void move_from_league_club_entrance_to_item_printer(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    context.wait_for_all_requests();

    // move forwards towards table next to item printer
    pbf_move_left_joystick(context, 120, 0, 200, 50);

    // look left towards item printer
    pbf_move_left_joystick(context, 0, 128, 10, 50);
}

void move_from_item_printer_to_material_farming(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Start moving from item printer to material farming.");
    move_from_item_printer_to_blueberry_entrance(env, context);
    fly_from_blueberry_to_north_province_3(env, context);
}

// assumes you start in the position in front of the item printer, as if you finished using it.
void move_from_item_printer_to_blueberry_entrance(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    context.wait_for_all_requests();

    // look left towards door
    pbf_move_left_joystick(context, 0, 128, 10, 50);

    // re-orient camera to look same direction as player
    pbf_press_button(context, BUTTON_L, 50, 50);

    // move forward towards door
    pbf_move_left_joystick(context, 128, 0, 700, 50);

    context.wait_for_all_requests();

    env.log("Wait for detection of Blueberry navigation menu.");

    // Wait for detection of Blueberry navigation menu
    ImageFloatBox select_entrance_box(0.031, 0.193, 0.047, 0.078);
    GradientArrowWatcher select_entrance(COLOR_RED, GradientArrowType::RIGHT, select_entrance_box);
    int ret = wait_until(env.console, context, Milliseconds(5000), { select_entrance }, Milliseconds(1000));
    if (ret == 0){
        env.log("Blueberry navigation menu detected.");
    }else{
        env.console.log("Failed to detect Blueberry navigation menu.");
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Failed to find the exit from the League room.",
            true
        );
    }

    // press A
    pbf_mash_button(context, BUTTON_A, 100);    

    // check for overworld
    OverworldWatcher overworld(COLOR_CYAN);  
    ret = wait_until(env.console, context, Milliseconds(10000), { overworld });
    if (ret == 0){
        env.log("Overworld detected");
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Failed to detect overworld.",
            true
        );      
    }
}


void fly_from_blueberry_to_north_province_3(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    int numAttempts = 0;
    int maxAttempts = 10;
    bool isFlySuccessful = false;

    while (!isFlySuccessful && numAttempts < maxAttempts){
        numAttempts++;

        // close all menus
        pbf_mash_button(context, BUTTON_B, 100);

        open_map_from_overworld(env.program_info(), env.console, context);

        // change from Blueberry map to Paldea map
        pbf_press_button(context, BUTTON_R, 50, 300);

        // zoom out
        pbf_press_button(context, BUTTON_ZL, 25, 200);

        // move cursor up-left
        pbf_move_left_joystick(context, 112, 0, 171, 50);

        // press A to fly to North province area 3
        isFlySuccessful = fly_to_overworld_from_map(env.program_info(), env.console, context, true);

        if (!isFlySuccessful){
            env.log("Failed to fly to North province area 3.");
        }
    }

    if (!isFlySuccessful){

        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Failed to fly to North province area 3, ten times in a row.",
            true
        );
        /* try{
            // try one last attempt, by using pokecenter detection

            // close all menus
            pbf_mash_button(context, BUTTON_B, 100);

            open_map_from_overworld(env.program_info(), env.console, context);

            // change from Blueberry map to Paldea map
            pbf_press_button(context, BUTTON_R, 50, 300);

            // zoom out
            pbf_press_button(context, BUTTON_ZL, 25, 200);

            // move up, but ending up far from other pokecenters
            pbf_move_left_joystick(context, 128, 0, 190, 50);

            // zoom back in to default zoom level
            pbf_press_button(context, BUTTON_ZR, 25, 200);

            fly_to_closest_pokecenter_on_map(env.program_info(), env.console, context);
        }
        catch(OperationFailedException& e){
            (void)e;
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to fly to North province area 3, five times in a row.",
                true
            );
        } */
    }
}



}
}
}
