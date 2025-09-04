/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BoxGenderDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BoxShinySymbolDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BoxNatureDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"
#include "PokemonSwSh_EggHelpers.h"
#include "PokemonSwSh_EggAutonomous.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh/Programs/PokemonSwSh_MenuNavigation.h"

namespace PokemonAutomation{

using namespace Pokemon;

namespace NintendoSwitch{
namespace PokemonSwSh{

namespace{

// We assume Pokemon app is always at row 0, col 1
const size_t POKEMON_APP_INDEX = 1;
// We assume Town Map app is always at row 1, col 0
const size_t TOWN_MAP_APP_INDEX = 5;


}


EggAutonomous_Descriptor::EggAutonomous_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggAutonomous",
        STRING_POKEMON + " SwSh", "Egg Autonomous",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggAutonomous.md",
        "Automatically fetch+hatch eggs and keep all shinies.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class EggAutonomous_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : m_hatched(m_stats["Eggs Hatched"])
        , m_errors(m_stats["Errors"])
        , m_fetch_attempts(m_stats["Fetch Attempts"])
        , m_fetch_success(m_stats["Fetch Success"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Eggs Hatched");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Fetch Attempts");
        m_display_order.emplace_back("Fetch Success");
        m_display_order.emplace_back("Shinies");
    }

    std::atomic<uint64_t>& m_hatched;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_fetch_attempts;
    std::atomic<uint64_t>& m_fetch_success;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> EggAutonomous_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


EggAutonomous::EggAutonomous()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b><br>Required to read IVs.",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , MAX_KEEPERS(
        "<b>Max Keepers:</b><br>Stop the program after keeping this many " + STRING_POKEMON + ". "
        "This number plus the number of " + STRING_POKEMON + " in the box left to your current box must not exceed 30. "
        "Otherwise, the program will break when that box is full.",
        LockMode::LOCK_WHILE_RUNNING,
        10, 1, 30
    )
    , LOOPS_PER_FETCH(
        "<b>Bike Loops Per Fetch:</b><br>Fetch an egg after doing this many bike loops on Route 5.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1
    )
    , NUM_EGGS_IN_COLUMN(
        "<b>Num Eggs in Column:</b><br>How many eggs already deposited in the first column in Box 1.",
        {
            {0, "0", "0"},
            {1, "1", "1"},
            {2, "2", "2"},
            {3, "3", "3"},
            {4, "4", "4"},
            {5, "5", "5"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , AUTO_SAVING(
        "<b>Auto-Saving:</b><br>Automatically save the game to recover from crashes and allow eggs to be unhatched.<br>"
        "(Unhatching eggs can be useful for obtaining breeding parents by rehatching a perfect egg in a game with a different language.)<br><br>"
        "To collect (unhatched) eggs with the desired stats, set this option to \"Save before every batch\". "
        "Then set the Action Table below to \"Stop Program\" on the desired stats. "
        "Once the program stops on the baby with the desired stats, you can manually reset the game and it will revert to an egg in your party.",
        {
            {AutoSave::NoAutoSave, "none", "No auto-saving. (No error/crash recovery.)"},
            {AutoSave::AfterStartAndKeep, "start-and-keep", "Save at beginning and after obtaining each baby that is kept. (Allows for error/crash recovery.)"},
            {AutoSave::EveryBatch, "every-batch", "Save before every batch. (Allows you to unhatch eggs.)"},
        },
          LockMode::LOCK_WHILE_RUNNING,
        AutoSave::AfterStartAndKeep
    )
    , FILTERS0(
        StatsHuntIvJudgeFilterTable_Label_Eggs,
        {
            .action = true,
            .shiny = true,
            .gender = true,
            .nature = true,
        }
    )
    , DEBUG_PROCESSING_HATCHED(
        "Debug the part of program after all eggs hatched",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_NONSHINY_KEEP(
        "Non-Shiny Keep",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_SHINY(
        "Shiny Hatch",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , m_notification_noop("", false, false)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_NONSHINY_KEEP,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);
    // PA_ADD_OPTION(STEPS_TO_HATCH);

    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(LOOPS_PER_FETCH);
    PA_ADD_OPTION(NUM_EGGS_IN_COLUMN);
    PA_ADD_OPTION(AUTO_SAVING);
    PA_ADD_OPTION(FILTERS0);
    PA_ADD_OPTION(NOTIFICATIONS);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(DEBUG_PROCESSING_HATCHED);
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
    }
}

void EggAutonomous::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();
    env.update_stats();

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    if (DEBUG_PROCESSING_HATCHED){ // only for debugging
        const bool need_taxi = true;
        process_hatched_pokemon(env, context, stats, need_taxi);
        return;
    }


    if (AUTO_SAVING == AutoSave::AfterStartAndKeep){
        save_game(env, context);
        m_num_eggs_in_storage_when_game_saved = static_cast<uint8_t>(NUM_EGGS_IN_COLUMN.current_value());
    }
    m_num_eggs_retrieved = static_cast<uint8_t>(NUM_EGGS_IN_COLUMN.current_value());

    m_num_pokemon_kept = 0;

    m_player_at_loop_start = false;

    // while(run_batch(env, context, stats)){}

    size_t consecutive_failures = 0;
    while(m_num_pokemon_kept < MAX_KEEPERS){
        try{
            if (TOUCH_DATE_INTERVAL.ok_to_touch_now()){
                env.log("Touching date to prevent rollover.");
                env.console.overlay().add_log("Touching date", COLOR_WHITE);
                pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
                touch_date_from_home(env.console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
                resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            }

            // Hatch one batch of eggs. If run_batch() returns true, stop the egg loop.
            if (run_batch(env, context, stats)){
                break;
            }
            env.log("stats: " + stats.to_str(StatsTracker::DISPLAY_ON_SCREEN));
            // We successfully finish one egg loop iteration without any error thrown.
            // So we reset the failure counter.
            consecutive_failures = 0;
        }catch (OperationFailedException& e){
            stats.m_errors++;
            env.update_stats();
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);
            
            if (SAVE_DEBUG_VIDEO){
                // Take a video to give more context for debugging
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                context.wait_for_all_requests();
            }

            // If there is no auto save, then we shouldn't reset to game to lose previous progress.
            if (AUTO_SAVING == AutoSave::NoAutoSave){
                throw;
            }

            consecutive_failures++;
            if (consecutive_failures >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed 3 batches in the row.",
                    env.console
                );
            }
            ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
            env.console.overlay().add_log("Reset game", COLOR_WHITE);
            reset_game_from_home_with_inference(
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );

            m_player_at_loop_start = false;
            m_num_eggs_retrieved = m_num_eggs_in_storage_when_game_saved;
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

// Run one iteration of the egg loop:
// - Hatch five eggs while fetch five eggs.
// - Check if pokemon needs to be kept. Keep them if needed.
// - Put five eggs from storage to party. Save game if needed.
// Return true if the egg loop should stop.
bool EggAutonomous::run_batch(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats
){
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    if (m_player_at_loop_start == false){ // reset position
        const bool fly_from_overworld = true; // fly from menu
        call_flying_taxi(env, context, fly_from_overworld);
    }

    size_t bike_loop_count = 0;
    const size_t MAX_BIKE_LOOP_COUNT = 100;
    size_t num_eggs_hatched = 0;
    m_player_at_loop_start = false;

    // Each iteration in the while-loop is made by:
    // - bike loops of LOOPS_PER_FETCH times. Bike loops begin at lady or nursery front door, end at lady.
    // - if not enough eggs fetched, talk to lady to try fetching an egg.
    while (num_eggs_hatched < 5 || m_num_eggs_retrieved < 5){
        // Detect when Y-Comm icon disappears. This is the time an egg is hatching
        const bool y_comm_visible_when_egg_hatching = false;
        YCommIconDetector egg_hatching_detector(y_comm_visible_when_egg_hatching);

        bool restart_bike_loop = false;
        for (size_t i_bike_loop = 0; i_bike_loop < this->LOOPS_PER_FETCH && bike_loop_count < MAX_BIKE_LOOP_COUNT;){
            context.wait_for_all_requests();
            // +1 here because video overlay is for general users. General users start counts at 1, while us programmers start count at 0.
            if (restart_bike_loop){
                env.console.overlay().add_log("Restart loop " + std::to_string(bike_loop_count+1), COLOR_WHITE);
                restart_bike_loop = false;
            }else{
                env.console.overlay().add_log("Loop " + std::to_string(bike_loop_count+1), COLOR_WHITE);
            }
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    travel_to_spin_location(context);
                    travel_back_to_lady(context);
                },
                {{egg_hatching_detector}}
            );

            if (ret < 0){ // we are at nursery lady; no egg hatching detected
                ++i_bike_loop;
                ++bike_loop_count;
                continue;
            }

            // Egg hatching
            do{
                ++num_eggs_hatched;
                stats.m_hatched++;
                env.update_stats();
                wait_for_egg_hatched(env, context, stats, num_eggs_hatched);
                if (num_eggs_hatched == 5){
                    // We hatched all five eggs. No more eggs can hatch. Go to next loop
                    break;
                }
                // Now we see if we can hatch one more egg.
                ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        // Try move a little to hatch more:
                        // We move toward lower-left so that it wont hit the lady or enter the Nursory.
                        pbf_move_left_joystick(context, 0, 255, 100, 10);
                    },
                    {{egg_hatching_detector}}
                );
            } while(ret == 0);
            // now no more hatching in this bike loop
            // We either cannot find a consecutive hatch any more or we already hatch five of them

            if (num_eggs_hatched == 5 && m_num_eggs_retrieved == 5){
                m_player_at_loop_start = false;
                break;
            }

            // Now we either cannot find a consecutive hatch any more, or we already hatch five for them, but
            // we still need to fetch more eggs

            // Use fly to reset the location because now we don't know where the player character is.
            const bool fly_from_overworld = true;
            call_flying_taxi(env, context, fly_from_overworld);
            restart_bike_loop = true;
            // We don't update i_bike_loop here because we haven't finished one full bike loop due to egg hatching
        } // end one bike loop

        if (bike_loop_count >= MAX_BIKE_LOOP_COUNT){
            env.log("Reached max number of bike loops " + std::to_string(MAX_BIKE_LOOP_COUNT));
            env.console.overlay().add_log("Error: max loops " + std::to_string(MAX_BIKE_LOOP_COUNT), COLOR_WHITE);
            env.log("Take a screenshot of party to debug.");
            // Now take a photo at the player's party for dumping debug info:
            // Enter Rotom Phone menu
            pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
            // Select Pokemon App
            navigate_to_menu_app(env, env.console, context, POKEMON_APP_INDEX, NOTIFICATION_ERROR_RECOVERABLE);
            // From menu enter Pokemon App
            ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);
            context.wait_for_all_requests();

            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Max number of loops reached. Not enough eggs in party?",
                env.console
            );
        }
        
        context.wait_for_all_requests();
        if (m_num_eggs_retrieved < 5){
            // Update num_eggs_retrieved
            m_num_eggs_retrieved = talk_to_lady_to_fetch_egg(env, context, stats, m_num_eggs_retrieved);
            if (num_eggs_hatched == 5 && m_num_eggs_retrieved == 5){
                m_player_at_loop_start = true;
                break;
            }
        }
    }

    // - Go to pokemon storage.
    // - Check the hatched pokemon, keep shiny pokemon and those that match the stats requirements. Release the rest.
    // - Retrieve the stored egg column to the party.
    // - Call flying taxi to reset player location if needed
    // Return true if the program should stop
    size_t last_num_pokemon_kept = m_num_pokemon_kept;
    if (process_hatched_pokemon(env, context, stats, !m_player_at_loop_start)){
        // While checking hatched pokemon, we find that We need to stop the program:
        return true;
    }
    m_num_eggs_retrieved = 0;
    // after process_hatched_pokemon(), the player location is at the start of bike loop
    m_player_at_loop_start = true;

    bool save = false;
    switch (AUTO_SAVING){
    case AutoSave::NoAutoSave:
        break;
    case AutoSave::AfterStartAndKeep:
        save = (last_num_pokemon_kept != m_num_pokemon_kept);
        break;
    case AutoSave::EveryBatch:
        save = true;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid saving option.");
    }

    if (save){
        save_game(env, context);
        m_num_eggs_in_storage_when_game_saved = 0;
    }
    
    context.wait_for_all_requests();
    return false;
}

void EggAutonomous::save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    env.log("Save game.");
    env.console.overlay().add_log("Save game", COLOR_WHITE);
    pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_button(context, BUTTON_R, 80ms, 2000ms);
    pbf_mash_button(context, BUTTON_A, 500ms);
    mash_B_until_y_comm_icon(env, context, "Cannot detect end of saving game.");
}

void EggAutonomous::call_flying_taxi(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    bool fly_from_overworld
){
    context.wait_for_all_requests();
    env.log("Fly to reset position");
    env.console.overlay().add_log("Call Flying Taxi", COLOR_WHITE);
    if (fly_from_overworld){
        // Open menu
        ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
    }

    navigate_to_menu_app(env, env.console, context, TOWN_MAP_APP_INDEX, NOTIFICATION_ERROR_RECOVERABLE);

    fly_home(context, false);
    mash_B_until_y_comm_icon(env, context, "Cannot detect end of flying taxi animation.");
}

void EggAutonomous::wait_for_egg_hatched(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats,
    size_t num_hatched_eggs
){
    env.console.overlay().add_log("Egg hatching " + std::to_string(num_hatched_eggs) + "/5", COLOR_GREEN);
    const bool y_comm_visible_at_end_of_egg_hatching = true;
    YCommIconDetector end_egg_hatching_detector(y_comm_visible_at_end_of_egg_hatching);
    const int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 60 * TICKS_PER_SECOND);
        },
        {{end_egg_hatching_detector}}
    );
    if (ret > 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Cannot detect egg hatching ends.",
            env.console
        );
    }
}

size_t EggAutonomous::talk_to_lady_to_fetch_egg(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats,
    size_t num_eggs_retrieved
){
    env.log("Fetching egg");
    stats.m_fetch_attempts++;
    env.update_stats();
    // collect_egg(context);
    RetrieveEggArrowFinder egg_arrow_detector(env.console);
    CheckNurseryArrowFinder no_egg_arrow_detector(env.console);

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (size_t i_hatched = 0; i_hatched < 2; i_hatched++){
                pbf_press_button(context, BUTTON_A, 20, 150);
            }
            pbf_wait(context, 200);
        },
        {
            egg_arrow_detector,
            no_egg_arrow_detector,
        }
    );
    
    const bool y_comm_visible_at_end_of_dialog = true;
    YCommIconDetector dialog_over_detector(y_comm_visible_at_end_of_dialog);
    switch (ret){
    case 0:
        ++num_eggs_retrieved;
        env.log("Found egg");
        env.console.overlay().add_log("Found egg " + std::to_string(num_eggs_retrieved) + "/5", COLOR_WHITE);
        stats.m_fetch_success++;
        env.update_stats();
        // Press A to get the egg
        ssf_press_button(context, BUTTON_A, 320ms, 160ms);

        ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 30s);
            },
            {{dialog_over_detector}}
        );
        break;

    case 1:
        env.log("No egg");
        env.console.overlay().add_log("No egg", COLOR_WHITE);
        run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 5s);
            },
            {{dialog_over_detector}}
        );
        return num_eggs_retrieved;
//        break;

    default:
        env.log("Daycare lady not found.");
        env.console.overlay().add_log("No daycare lady", COLOR_WHITE);
        return num_eggs_retrieved;
//        OperationFailedException::fire(
//            ErrorReport::SEND_ERROR_REPORT,
//            "Cannot detect dialog selection arrow when talking to Nursery lady.",
//            env.console
//        );
    }

    // If dialog over is not detected:
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Cannot detect end of Nursery lady dialog. No Y-Comm mark found.",
            env.console
        );
    }

    return num_eggs_retrieved;
}

// After all five eggs hatched and another five eggs deposit into the first column of the box,
// call this function to:
// - Go to pokemon storage.
// - Check the hatched pokemon, keep shiny pokemon and those that match the stats requirements. Release the rest.
// - Retrieve the stored egg column to the party.
// - Call flying taxi to reset player location if needed
// Return true if the program should stop
bool EggAutonomous::process_hatched_pokemon(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats,
    bool need_taxi
){
    env.log("Checking hatched pokemon.");
    env.console.overlay().add_log("Checking hatched pokemon", COLOR_WHITE);

    // Press X to open menu
    ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);

    navigate_to_menu_app(env, env.console, context, POKEMON_APP_INDEX, NOTIFICATION_ERROR_RECOVERABLE);

    const Milliseconds BOX_CHANGE_DELAY = GameSettings::instance().BOX_CHANGE_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;

    // From menu enter Pokemon App
    ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);
    // From Pokemon App to storage box
    ssf_press_button(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY0, EGG_BUTTON_HOLD_DELAY);
    // Move left down to the first hatched pokemon in the party
    box_scroll(context, DPAD_LEFT);
    box_scroll(context, DPAD_DOWN);

    context.wait_for_all_requests();
    {
        // Define the scope of video overlay rendering for various checks:
        VideoOverlaySet overlay_set(env.console.overlay());
        BoxShinySymbolDetector::make_overlays(overlay_set);
        BoxGenderDetector gender_detector;
        gender_detector.make_overlays(overlay_set);
        IvJudgeReaderScope iv_reader(env.console.overlay(), LANGUAGE);
        BoxNatureDetector nature_detector(env.console.overlay());

        for (size_t i_hatched = 0; i_hatched < 5; i_hatched++){
            pbf_wait(context, 50); // wait for a while to make sure the pokemon stats are loaded.
            context.wait_for_all_requests();
            auto screen = env.console.video().snapshot();

            bool shiny = BoxShinySymbolDetector::detect(screen);
            if (shiny){
                env.log("Pokemon " + std::to_string(i_hatched) + " is shiny!", COLOR_BLUE);
                env.console.overlay().add_log("Pokemon " + std::to_string(i_hatched+1) + "/5 is shiny!", COLOR_YELLOW);
                stats.m_shinies++;
                env.update_stats();
                send_encounter_notification(
                    env,
                    m_notification_noop,
                    NOTIFICATION_SHINY,
                    false, true, {{{}, ShinyType::UNKNOWN_SHINY}}, std::nan(""),
                    screen
                );
            }else{
                env.log("Pokemon " + std::to_string(i_hatched) + " is not shiny.", COLOR_PURPLE);
                env.console.overlay().add_log("Pokemon " + std::to_string(i_hatched+1) + "/5 not shiny", COLOR_WHITE);
            }
            // Note: we assume the pokemon storage UI is in the state of judging pokemon stats.
            //   In this way we can detect pokemon stats.
            
            IvJudgeReader::Results IVs = iv_reader.read(env.console, screen);
            StatsHuntGenderFilter gender = gender_detector.detect(screen);
            env.log(IVs.to_string(), COLOR_GREEN);
            env.log("Gender: " + gender_to_string(gender), COLOR_GREEN);
            NatureReader::Results nature = nature_detector.read(env.console.logger(), screen);

            StatsHuntAction action = FILTERS0.get_action(shiny, gender, nature.nature, IVs);

            auto send_keep_notification = [&](){
                if (!shiny){
                    send_encounter_notification(
                        env,
                        NOTIFICATION_NONSHINY_KEEP,
                        NOTIFICATION_SHINY,
                        false, false, {}, std::nan(""),
                        screen
                    );
                }
            };
            switch (action){
            case StatsHuntAction::StopProgram:
                env.log("Program stop requested...");
                env.console.overlay().add_log("Request program stop", COLOR_WHITE);
                send_keep_notification();
                return true;
            case StatsHuntAction::Keep:
                env.log("Moving Pokemon to keep box...", COLOR_BLUE);
                m_num_pokemon_kept++;
                env.console.overlay().add_log("Keep pokemon " + std::to_string(m_num_pokemon_kept) + "/" + std::to_string(MAX_KEEPERS), COLOR_YELLOW);
                send_keep_notification();

                // Press A twice to pick the pokemon
                ssf_press_button_ptv(context, BUTTON_A, 480ms, EGG_BUTTON_HOLD_DELAY);
                ssf_press_button_ptv(context, BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);

                // Move it rightward, so that it stays on top of the box area
                box_scroll(context, DPAD_RIGHT);
                // Press Button L to change to the box on the left
                ssf_press_button_ptv(context, BUTTON_L, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);

                // Because we don't know which place in the box to place the pokemon, we will
                // throw the pokemon in the all-box view. So it automatically sit in the first empty slot
                // in the box:
                
                // Move it three times upward, so that it stays on top of the "Box List" button
                box_scroll(context, DPAD_UP);
                box_scroll(context, DPAD_UP);
                box_scroll(context, DPAD_UP);
                
                // Press the button to go to box list view
                ssf_press_button_ptv(context, BUTTON_A, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
                // Press button A to drop the pokemon into the box
                ssf_press_button_ptv(context, BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);
                // Press button B to go back to the last box
                ssf_press_button_ptv(context, BUTTON_B, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
                // Press button R to change to the box on the right, the box with the next batch of eggs
                ssf_press_button_ptv(context, BUTTON_R, BOX_CHANGE_DELAY, EGG_BUTTON_HOLD_DELAY);
                // Move cursor left to point to the last slot in the party
                box_scroll(context, DPAD_LEFT);
                // Move cursor downward three times so that it goes to the original place (second slot in the party)
                box_scroll(context, DPAD_DOWN);
                box_scroll(context, DPAD_DOWN);
                box_scroll(context, DPAD_DOWN);
                
                if (m_num_pokemon_kept >= MAX_KEEPERS){
                    env.log("Max keepers reached. Stopping program...");
                    env.console.overlay().add_log("Max Keepers reached.", COLOR_WHITE);
                    return true;
                }
                break;
            case StatsHuntAction::Discard:
                env.log("Releasing Pokemon...", COLOR_PURPLE);
                env.console.overlay().add_log("Release Pokemon", COLOR_WHITE);

                // ssf_press_button2(context, BUTTON_A, 60, 10);
                // ssf_press_dpad2(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
                // ssf_press_dpad2(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
                // ssf_press_button2(context, BUTTON_A, 125, 10);
                // ssf_press_dpad2(context, DPAD_UP, BOX_SCROLL_DELAY, EGG_BUTTON_HOLD_DELAY);
                // pbf_mash_button(context, BUTTON_A, 180);

                // Press A to open pokemon menu
                pbf_press_button(context, BUTTON_A, 20, 50);
                context.wait_for_all_requests();
                StoragePokemonMenuArrowFinder pokemon_menu_detector(env.console.overlay());
                int ret = wait_until(
                    env.console, context, std::chrono::seconds(10),
                    {{pokemon_menu_detector}}
                );
                if (ret != 0){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Cannot detect pokemon menu in storage box.",
                        env.console
                    );
                }

                const bool stop_on_detected = true;
                BlackDialogBoxDetector dialog_detector(stop_on_detected);
                VideoOverlaySet dialog_overlay_set(env.console);
                dialog_detector.make_overlays(dialog_overlay_set);

                // Move cursor upward two times to point to "Release" menu item
                pbf_press_dpad(context, DPAD_UP, 20, 20);
                pbf_press_dpad(context, DPAD_UP, 20, 20);

                // Press A to release
                pbf_press_button(context, BUTTON_A, 20, 105);
                // Move cursor from "Not release" to "release".
                pbf_press_dpad(context, DPAD_UP, 20, 30);
                // Press A to confirm release, wait for a while to let the next dialog box pop up.
                pbf_press_button(context, BUTTON_A, 20, 200);

                context.wait_for_all_requests();
                ret = wait_until(
                    env.console, context, std::chrono::seconds(10),
                    {{dialog_detector}}
                );
                if (ret != 0){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Miss second dialog when releasing pokemon.",
                        env.console
                    );
                }
                pbf_press_button(context, BUTTON_A, 20, 100);
                
                size_t dialog_count = 0;
                const size_t max_dialog_count = 6;
                for (; dialog_count < max_dialog_count; dialog_count++){
                    context.wait_for_all_requests();
                    if (!dialog_detector.process_frame(env.console.video().snapshot(), current_time())){
                        break;
                    }
                    pbf_press_button(context, BUTTON_A, 20, 100);
                }
                if (dialog_count == max_dialog_count){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Unexpected dialogs when releasing pokemon.",
                        env.console
                    );
                }
                break;
            }
        }
    }

    // Get eggs to party:

    // Move cursor to the first slot in the box
    box_scroll(context, DPAD_UP);
    box_scroll(context, DPAD_RIGHT);

    // Press Y twice to change selection method to group selection
    pbf_press_button(context, BUTTON_Y, EGG_BUTTON_HOLD_DELAY, 400ms);
    pbf_press_button(context, BUTTON_Y, EGG_BUTTON_HOLD_DELAY, 400ms);

    // Press A to start selection
    pbf_press_button(context, BUTTON_A, EGG_BUTTON_HOLD_DELAY, 400ms);
    // Move down to selection the entire column
    for (size_t c = 0; c < 4; c++){
        box_scroll(context, DPAD_DOWN);
    }
    // Press A to finish the selection
    ssf_press_button_ptv(context, BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);
    
    // Move cursor to the second slot in the party
    box_scroll(context, DPAD_LEFT);
    box_scroll(context, DPAD_DOWN);

    // Press A to finish dropping the egg column 
    ssf_press_button_ptv(context, BUTTON_A, BOX_PICKUP_DROP_DELAY, EGG_BUTTON_HOLD_DELAY);

    // leave pokemon box, back to pokemon app
    ssf_press_button(context, BUTTON_B, GameSettings::instance().BOX_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);
    //  Back out to menu.
    ssf_press_button(context, BUTTON_B, GameSettings::instance().POKEMON_TO_MENU_DELAY0, EGG_BUTTON_HOLD_DELAY);

    if (need_taxi){
        bool fly_from_overworld = false; // fly from menu
        call_flying_taxi(env, context, fly_from_overworld);
    }else{
        // Leave menu, go back to overworld
        const bool y_comm_visible = true;
        YCommIconDetector y_comm_detector(y_comm_visible);
        const int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 5000ms);
            },
            {{y_comm_detector}}
        );
        if (ret > 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Cannot detect Y-Comm after leaving menu.",
                env.console
            );
        }
    }

    return false;
}

void EggAutonomous::mash_B_until_y_comm_icon(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const std::string& error_msg
){
    context.wait_for_all_requests();
    const bool y_comm_visible = true;
    YCommIconDetector y_comm_detector(y_comm_visible);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 10s);
        },
        {y_comm_detector}
    );
    if (ret != 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            error_msg + " No Y-Comm mark found.",
            env.console
        );
    }
}




}
}
}

