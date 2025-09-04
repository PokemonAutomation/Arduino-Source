/*  Shiny Hunt - Scatterbug
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV_LetsGoTools.h"
#include "PokemonSV_ShinyHunt-Scatterbug.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



ShinyHuntScatterbug_Descriptor::ShinyHuntScatterbug_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ShinyHuntScatterbug",
        STRING_POKEMON + " SV", "Shiny Hunt - Scatterbug",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ShinyHunt-Scatterbug.md",
        "Shiny hunt Scatterbug.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ShinyHuntScatterbug_Descriptor::Stats : public LetsGoEncounterBotStats{
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
std::unique_ptr<StatsTracker> ShinyHuntScatterbug_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




ShinyHuntScatterbug::ShinyHuntScatterbug()
    : SAVE_GAME_AT_START(
        "<b>Save Game at Program Start:</b><br>"
        "This is to ensure the program can continue after resetting the game. Uncheck this option if you have manually saved the game.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , SANDWICH_OPTIONS(
        "Sandwich Options",
        &LANGUAGE,
        BaseRecipe::shiny,
        false,
        GroupOption::EnableMode::ALWAYS_ENABLED
    )
    , GO_HOME_WHEN_DONE(true)
    , AUTO_HEAL_PERCENT(
        "<b>Auto-Heal %</b><br>Auto-heal if your HP drops below this percentage.",
        LockMode::UNLOCK_WHILE_RUNNING,
        75, 0, 100
    )
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b><br>"
        "Set this on to save a Switch video everytime an error occurs. You can send the video to developers to help them debug later.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , DEBUG_WARP_TO_POKECENTER(
        "<b>Whether to change the program to just warping to closest PokeCenter and stopping:</b><br>"
        "This is for debugging the PokeCenter warping function.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SKIP_SANDWICH(
        "<b>Whether to skip making sandwich:</b><br>"
        "This is for debugging the program without waiting for sandwich making.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SAVE_GAME_AT_START);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(AUTO_HEAL_PERCENT);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
        PA_ADD_OPTION(DEBUG_WARP_TO_POKECENTER);
        PA_ADD_OPTION(SKIP_SANDWICH);
    }

    PA_ADD_OPTION(NOTIFICATIONS);
}


void ShinyHuntScatterbug::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHuntScatterbug_Descriptor::Stats& stats = env.current_stats<ShinyHuntScatterbug_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 50);

    assert_16_9_720p_min(env.logger(), env.console);

    if (DEBUG_WARP_TO_POKECENTER){
        reset_to_pokecenter(env.program_info(), env.console, context);
        return;
    }

    size_t consecutive_failures = 0;
    m_pending_save = false;

    if (SAVE_GAME_AT_START){
        save_game_from_overworld(env.program_info(), env.console, context);
    }

    OverworldSensors sensors(
        env.logger(), env.console, context
    );
    m_sensors = &sensors;

    OverworldBattleTracker battle_tracker(env.logger(), sensors);
    m_battle_tracker = &battle_tracker;

    LetsGoEncounterBotTracker encounter_tracker(
        env, env.console,
        stats,
        sensors.lets_go_kill
    );
    m_encounter_tracker = &encounter_tracker;

    while(true){
        try{
            run_one_sandwich_iteration(env, context);
            consecutive_failures = 0;
        }catch(OperationFailedException& e){
            stats.m_errors++;
            env.update_stats();
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            if (SAVE_DEBUG_VIDEO){
                // Take a video to give more context for debugging
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                context.wait_for_all_requests();
            }

            if (m_pending_save){
                // We have found a pokemon to keep, but before we can save the game to protect the pokemon, an error occurred.
                // To not lose the pokemon, don't reset.
                env.log("Found an error before we can save the game to protect the newly kept pokemon.", COLOR_RED);
                env.log("Don't reset game to protect it.", COLOR_RED);
                throw std::move(e);
            }

            consecutive_failures++;
            if (consecutive_failures >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed 3 times in the row.",
                    env.console
                );
            }

            env.log("Reset game to handle recoverable error");
            reset_game(env.program_info(), env.console, context);
            ++stats.m_game_resets;
            env.update_stats();

        }catch(ProgramFinishedException&){
            GO_HOME_WHEN_DONE.run_end_of_program(context);
            throw;
        }
    }
}


// This function wraps around an action (e.g. go out of PokeCenter to make a sandwich) so that
// we can handle pokemon wild encounters when executing the action.
// Whenever a battle happens, we check shinies and handle battle according to user setting. After battle ends, move
// back to PokeCenter to start the `action` again.
// `action` must be an action starting at the PokeCenter
void ShinyHuntScatterbug::handle_battles_and_back_to_pokecenter(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    std::function<void(SingleSwitchProgramEnvironment& env, ProControllerContext& context)>&& action
){
    if (m_encounter_tracker == nullptr){
        throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "m_encounter_tracker == nullptr");
    }

    ShinyHuntScatterbug_Descriptor::Stats& stats = env.current_stats<ShinyHuntScatterbug_Descriptor::Stats>();

    bool action_finished = false;
    bool first_iteration = true;
    // a flag for the case that the action has finished but not yet returned to pokecenter
    bool returned_to_pokecenter = false;
    while(action_finished == false || returned_to_pokecenter == false){
        // env.console.overlay().add_log("Calculate what to do next");
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
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
                }else{
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
        m_sensors->throw_if_no_sound();
        if (ret >= 0){
            env.console.log("Detected battle.", COLOR_PURPLE);
            env.console.overlay().add_log("Detected battle");
            stats.m_encounters++;
            env.update_stats();
            m_encounter_tracker->encounter_rate_tracker().report_encounter();
            try{
                bool caught, should_save;
                process_battle(
                    caught, should_save,
                    env,
                    ENCOUNTER_BOT_OPTIONS,
                    env.console, context,
                    *m_battle_tracker,
                    m_encounter_tracker->encounter_frequencies(),
                    stats.m_shinies,
                    LANGUAGE
                );

                m_pending_save |= should_save;
            }catch (ProgramFinishedException&){
                GO_HOME_WHEN_DONE.run_end_of_program(context);
                throw;
            }
        }
        // Back on the overworld.
    } // end while(action_finished == false || returned_to_pokecenter == false)
}

// Start at Mesagoza South Gate pokecenter, make a sandwich, then use let's go repeatedly until 30 min passes.
// If 
void ShinyHuntScatterbug::run_one_sandwich_iteration(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    ShinyHuntScatterbug_Descriptor::Stats& stats = env.current_stats<ShinyHuntScatterbug_Descriptor::Stats>();

    bool saved_after_this_sandwich = false;

    WallClock last_sandwich_time = WallClock::min();

    auto save_if_needed = [&](){
        if (m_pending_save){
            save_game_from_overworld(env.program_info(), env.console, context);
            m_pending_save = false;
            saved_after_this_sandwich = true;
        }
    };

    handle_battles_and_back_to_pokecenter(
        env, context,
        [this, &last_sandwich_time](SingleSwitchProgramEnvironment& env, ProControllerContext& context){
            // Orient camera to look at same direction as player character
            // This is needed because when save-load the game, the camera is reset
            // to this location.
            pbf_press_button(context, BUTTON_L, 50, 40);
            // Move forward
            pbf_move_left_joystick(context, 128, 0, 180, 0);
            if (!SKIP_SANDWICH){
                picnic_from_overworld(env.program_info(), env.console, context);

                pbf_move_left_joystick(context, 128, 0, 30, 40);
                enter_sandwich_recipe_list(env.program_info(), env.console, context);
                make_sandwich_option(env, env.console, context, SANDWICH_OPTIONS);
                last_sandwich_time = current_time();
                leave_picnic(env.program_info(), env.console, context);
            }else{
                last_sandwich_time = current_time();
            }
        }
    );
    env.console.overlay().add_log("Started Let's Go Paths");
    save_if_needed();

    // Which path to choose starting at the PokeCenter.
    size_t path_id = 0;
    const size_t num_paths = 2;

    LetsGoHpWatcher hp_watcher(COLOR_RED);
    // In each iteration of this while-loop, it picks a path starting from the pokecenter or the
    // last sandwich making spot, use Let's Go along the path, then fly back to pokecenter.
    for (;;path_id = (path_id + 1) % num_paths){
        if (last_sandwich_time + std::chrono::minutes(30) < current_time()){
            env.log("Sandwich expires.");
            env.console.overlay().add_log("Sandwich expires");
            break;
        }

        env.console.log("Starting Let's Go hunting path " + std::to_string(path_id) + "...", COLOR_PURPLE);
        env.console.overlay().add_log("Let's Go on Path " + std::to_string(path_id));

        double hp = hp_watcher.last_known_value() * 100;
        if (0 < hp){
            env.console.log("Last Known HP: " + tostr_default(hp) + "%", COLOR_BLUE);
        }else{
            env.console.log("Last Known HP: ?", COLOR_RED);
        }
        if (0 < hp && hp < AUTO_HEAL_PERCENT){
            auto_heal_from_menu_or_overworld(env.program_info(), env.console, context, 0, true);
            stats.m_autoheals++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        }

        handle_battles_and_back_to_pokecenter(
            env, context,
            [this, &path_id, &hp_watcher](SingleSwitchProgramEnvironment& env, ProControllerContext& context){
                run_until<ProControllerContext>(
                    env.console, context,
                    [&](ProControllerContext& context){
                        run_lets_go_iteration(env, context, path_id);
                    },
                    {hp_watcher}
                );
            } // end [](...)
        ); // end handle_battles_and_back_to_pokecenter()
        save_if_needed();
    } // end for (;;path_id = (path_id + 1) % num_paths)

    if (!saved_after_this_sandwich){
        // Reset game to save rare herbs
        reset_game(env.program_info(), env.console, context);
        ++stats.m_game_resets;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
}

// One iteration of the hunt: 
// start at Mesagoza South Gate pokecenter, go out and use Let's Go to battle Scatterbug, 
void ShinyHuntScatterbug::run_lets_go_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t path_id){
    auto& console = env.console;
    // Orient camera to look at same direction as player character
    // This is needed because when save-load the game, the camera is reset
    // to this location.
    pbf_press_button(context, BUTTON_L, 50, 40);

    const bool throw_ball_if_bubble = true;

    auto move_forward_with_lets_go = [&](int num_iterations){
        context.wait_for_all_requests();
        for(int i = 0; i < num_iterations; i++){
            use_lets_go_to_clear_in_front(console, context, *m_encounter_tracker, throw_ball_if_bubble, [&](ProControllerContext& context){
                // Do the following movement while the Let's Go pokemon clearing wild pokemon.
                // Slowly Moving forward
                pbf_move_left_joystick(context, 128, 105, 800, 0);
            });
        }
    };

    if (path_id == 0){
        // move rightward, to the west
        pbf_move_left_joystick(context, 255, 128, 100, 20);
        // Align camera
        pbf_press_button(context, BUTTON_L, 50, 40);

        move_forward_with_lets_go(10);
    }else{ // path_id == 1
        // move leftward, to the east
        pbf_move_left_joystick(context, 0, 128, 100, 20);
        // Align camera
        pbf_press_button(context, BUTTON_L, 50, 40);

        move_forward_with_lets_go(5);

        // move rightward, to south
        pbf_move_left_joystick(context, 255, 128, 50, 20);
        // Align camera
        pbf_press_button(context, BUTTON_L, 50, 40);

        move_forward_with_lets_go(5);
    }
}


}
}
}
