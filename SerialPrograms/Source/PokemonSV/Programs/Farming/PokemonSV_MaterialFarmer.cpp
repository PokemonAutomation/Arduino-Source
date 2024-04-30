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
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV_MaterialFarmer.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_ShinyHunt-Scatterbug.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



MaterialFarmer_Descriptor::MaterialFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:MaterialFarmer",
        STRING_POKEMON + " SV", "Material Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/MaterialFarmer.md",
        "Farm materials - Happiny dust from Chanseys/Blisseys, for Item Printer.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct MaterialFarmer_Descriptor::Stats : public LetsGoEncounterBotStats{
    Stats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_autoheals(m_stats["Auto Heals"])
        , m_game_resets(m_stats["Game Resets"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.insert(m_display_order.begin() + 2, {"Sandwiches", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 3, {"Auto Heals", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 4, {"Game Resets", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 5, {"Errors", HIDDEN_IF_ZERO});
    }
    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_autoheals;
    std::atomic<uint64_t>& m_game_resets;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> MaterialFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MaterialFarmer::MaterialFarmer()
    : SAVE_GAME_BEFORE_SANDWICH(
        "<b>Save Game before each sandwich:</b><br>"
        "Recommended to leave on, as the sandwich maker will reset the game if it detects an error.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NUM_SANDWICH_ROUNDS(
          "<b>Number of sandwich rounds to run:</b><br>"
          "400-650 Happiny dust per sandwich, with Normal Encounter power level 2.<br>"
          "(e.g. Chorizo x4, Banana x2, Mayo x3, Whipped Cream x1)",
          LockMode::UNLOCK_WHILE_RUNNING,
          3
    )
    , LANGUAGE(
        "<b>Game Language:</b><br>Required to read sandwich ingredients.",
        IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SANDWICH_OPTIONS(LANGUAGE)
    , GO_HOME_WHEN_DONE(true)
    , AUTO_HEAL_PERCENT(
        "<b>Auto-Heal %</b><br>Auto-heal if your HP drops below this percentage.",
        LockMode::UNLOCK_WHILE_RUNNING,
        75, 0, 100
    )
    , SAVE_DEBUG_VIDEO(
        "<b>DEV MODE: Save debug videos to Switch:</b><br>"
        "Set this on to save a Switch video everytime an error occurs. You can send the video to developers to help them debug later.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SKIP_WARP_TO_POKECENTER(
        "<b>DEV MODE: Skip warping to closest PokeCenter:</b><br>"
        "This is for debugging the program without waiting for the initial warp.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SKIP_SANDWICH(
        "<b>DEV MODE: Skip making sandwich:</b><br>"
        "This is for debugging the program without waiting for sandwich making.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
        PA_ADD_OPTION(SKIP_WARP_TO_POKECENTER);
        PA_ADD_OPTION(SKIP_SANDWICH);
    }
    PA_ADD_OPTION(SAVE_GAME_BEFORE_SANDWICH);
    PA_ADD_OPTION(NUM_SANDWICH_ROUNDS);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(AUTO_HEAL_PERCENT);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void MaterialFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    


    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 50);

    // Throw user setup errors early in program
    // - Ensure language is set
    const Language language = LANGUAGE;
    if (language == Language::None) {
        throw UserSetupError(env.console.logger(), "Must set game language option to read ingredient lists.");
    }

    // - Ensure audio input is enabled
    LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
    wait_until(
        env.console, context,
        std::chrono::milliseconds(1100),
        {
            static_cast<AudioInferenceCallback&>(audio_detector)
        }
    );
    audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

    // start by warping to pokecenter for positioning reasons
    if (!SKIP_WARP_TO_POKECENTER){
        reset_to_pokecenter(env.program_info(), env.console, context);
    }

    MaterialFarmerOptions options {
        SAVE_GAME_BEFORE_SANDWICH,
        NUM_SANDWICH_ROUNDS,
        LANGUAGE,
        SANDWICH_OPTIONS,
        GO_HOME_WHEN_DONE, 
        AUTO_HEAL_PERCENT,
        SAVE_DEBUG_VIDEO, 
        SKIP_WARP_TO_POKECENTER,
        SKIP_SANDWICH,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATIONS
    };

    run_material_farmer(env, context, options);
    
}



void run_material_farmer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, MaterialFarmerOptions options){
    
    MaterialFarmer_Descriptor::Stats& stats = env.current_stats<MaterialFarmer_Descriptor::Stats>();

    LetsGoEncounterBotTracker encounter_tracker(
        env, env.console, context,
        stats,
        options.language_option
    );


    size_t consecutive_failures;
    bool done_sandwich_iteration;
    for (uint16_t i = 0; i < options.num_sandwich_rounds_option; i++){
        consecutive_failures = 0;
        done_sandwich_iteration = false;
        while (!done_sandwich_iteration){
            try{
                run_one_sandwich_iteration(env, context, encounter_tracker, options);
                done_sandwich_iteration = true;
            }catch(OperationFailedException& e){
                stats.m_errors++;
                env.update_stats();
                e.send_notification(env, options.notification_error_recoverable_option);

                if (options.save_debug_video_option){
                    // Take a video to give more context for debugging
                    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                }

                consecutive_failures++;
                if (consecutive_failures >= 3){
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, env.console,
                        "Failed 3 times in the row.",
                        true
                    );
                }

                env.log("Reset game to handle recoverable error");
                reset_game(env.program_info(), env.console, context);
                ++stats.m_game_resets;
                env.update_stats();
            }catch(ProgramFinishedException&){
                options.go_home_when_done_option.run_end_of_program(context);
                throw;
            }
        }
    }
}



// start at North Province (Area 3) Pokecenter, make a sandwich, then use let's go repeatedly until 30 min passes.
void run_one_sandwich_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker, MaterialFarmerOptions& options)
{

    MaterialFarmer_Descriptor::Stats& stats = env.current_stats<MaterialFarmer_Descriptor::Stats>();

    WallClock last_sandwich_time = WallClock::min();

    if (options.save_game_before_sandwich_option){
        save_game_from_overworld(env.program_info(), env.console, context);
    }
    // make sandwich then go back to Pokecenter to reset position
    if (!options.skip_sandwich_option){
        run_from_battles_and_back_to_pokecenter(env, context, 
            [&last_sandwich_time, &options](SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
                make_sandwich_option(env, env.console, context, options.sandwich_options);
                last_sandwich_time = current_time();
                leave_picnic(env.program_info(), env.console, context);

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
        
        auto sandwich_time_left =  std::chrono::minutes(30) + std::chrono::duration_cast<std::chrono::minutes>(last_sandwich_time - current_time());
        env.console.log(
            "Time left on sandwich: " + 
            std::to_string(sandwich_time_left.count()) + " min", 
            COLOR_PURPLE);

        if (is_sandwich_expired(last_sandwich_time)){
            env.log("Sandwich expired. Start another sandwich round.");
            env.console.overlay().add_log("Sandwich expired.");
            break;
        }

        // heal before starting Let's go
        env.console.log("Heal before starting Let's go", COLOR_PURPLE);
        double hp = hp_watcher.last_known_value() * 100;
        if (0 < hp){
            env.console.log("Last Known HP: " + tostr_default(hp) + "%", COLOR_BLUE);
        }else{
            env.console.log("Last Known HP: ?", COLOR_RED);
        }
        if (0 < hp && hp < options.auto_heal_percent_option){
            auto_heal_from_menu_or_overworld(env.program_info(), env.console, context, 0, true);
            stats.m_autoheals++;
            env.update_stats();
            send_program_status_notification(env, options.notification_status_update_option);
        }

        /*
        - Starts from pokemon center.
        - Flies to start position. Runs a Let's Go iteration. 
        - Then returns to pokemon center 
        */
        env.console.log("Starting Let's Go hunting path", COLOR_PURPLE);
        run_from_battles_and_back_to_pokecenter(env, context, 
            [&hp_watcher, &last_sandwich_time, &encounter_tracker](SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
                        if (is_sandwich_expired(last_sandwich_time)){
                            env.log("Sandwich expired. Return to Pokecenter.");
                            return;
                        }                        
                        move_to_start_position_for_letsgo1(env, context);
                        run_lets_go_iteration(env, context, encounter_tracker);
                    },
                    {hp_watcher}
                );
            } 
        ); 
        
        context.wait_for_all_requests();
    }


}

bool is_sandwich_expired(WallClock last_sandwich_time){
    return last_sandwich_time + std::chrono::minutes(30) < current_time();
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


// One iteration of the hunt: 
// start at North Province (Area 3) pokecenter, go out and use Let's Go to battle , 
void run_lets_go_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker)
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
    const int total_iterations = 13;

    context.wait_for_all_requests();
    for(int i = 0; i < total_iterations; i++){
        use_lets_go_to_clear_in_front(console, context, *m_encounter_tracker, throw_ball_if_bubble, [&](BotBaseContext& context){
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


}
}
}
