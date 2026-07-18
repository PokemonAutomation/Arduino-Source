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
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
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


}


EggAutonomous_Descriptor::EggAutonomous_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggAutonomous",
        STRING_POKEMON + " SwSh", "Egg Autonomous",
        "Programs/PokemonSwSh/EggAutonomous.html",
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
        "<b>Num Eggs in Column 1:</b><br>How many eggs already deposited in the first column in Box 1.",
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
    , NUM_EGGS_IN_PARTY(
        "<b>Num Eggs in Party:</b><br>Number of eggs in your party.",
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
    PA_ADD_OPTION(NUM_EGGS_IN_PARTY);
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
        resume_game_back_out(
            env.console,
            context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST,
            3200ms
        );
    }else{
        //  Connect the controller.
        require_player(env.console, context, BUTTON_B);
    }

    if (DEBUG_PROCESSING_HATCHED){ // only for debugging
        const bool need_taxi = true;
        process_hatched_pokemon(env, context, stats, need_taxi);
        return;
    }


    if (AUTO_SAVING == AutoSave::AfterStartAndKeep){
        save_game(env.console, context);
        m_num_eggs_in_storage_when_game_saved = static_cast<uint8_t>(NUM_EGGS_IN_COLUMN.current_value());
        m_num_eggs_in_party_when_game_saved = static_cast<uint8_t>(NUM_EGGS_IN_PARTY.current_value());
    }
    m_num_eggs_retrieved = static_cast<uint8_t>(NUM_EGGS_IN_COLUMN.current_value());
    m_num_eggs_in_party_at_batch_start = static_cast<uint8_t>(NUM_EGGS_IN_PARTY.current_value());

    m_num_pokemon_kept = 0;

    m_player_at_loop_start = false;

    // while(run_batch(env, context, stats)){}

    size_t consecutive_failures = 0;
    while(m_num_pokemon_kept < MAX_KEEPERS){
        try{
            if (TOUCH_DATE_INTERVAL.ok_to_touch_now()){
                env.log("Touching date to prevent rollover.");
                env.console.overlay().add_log("Touching date", COLOR_WHITE);
                go_home(env.console, context);
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
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 2000ms);
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
            go_home(env.console, context);
            env.console.overlay().add_log("Reset game", COLOR_WHITE);
            reset_game_from_home_with_inference(
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );

            m_player_at_loop_start = false;
            m_num_eggs_retrieved = m_num_eggs_in_storage_when_game_saved;
            m_num_eggs_in_party_at_batch_start = m_num_eggs_in_party_when_game_saved;
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

    size_t total_eggs_to_fetch = 10 - m_num_eggs_retrieved - m_num_eggs_in_party_at_batch_start;

    EggAutoPhase phase = EggAutoPhase::BIKE_LOOP;
    if (m_player_at_loop_start == false){ // reset position
        phase = EggAutoPhase::FLY_RESET;
    }

    size_t total_bike_loop_count = 0;
    const size_t MAX_BIKE_LOOP_COUNT = 100;
    size_t num_loops_since_last_fetch_attempt = 0;

    size_t num_eggs_hatched = 0;
    m_player_at_loop_start = false;
    
    // Each iteration in the while-loop is made by:
    // - bike loops of LOOPS_PER_FETCH times. Bike loops begin at lady or nursery front door, end at lady.
    // - if not enough eggs fetched, talk to lady to try fetching an egg.
    while (num_eggs_hatched < 5 || m_num_eggs_retrieved < total_eggs_to_fetch){

        // NOTE: the egg hatching detector cannot be constantly running, 
        // since it only detects the black dialog box, and speaking to the lady will also produce a black dialog box.
        // therefore, each phase has its own egg hatching detector, so it can be more easily turned on and off.

        switch(phase){
        case EggAutoPhase::BIKE_LOOP:{
            env.console.overlay().add_log("Loop " + std::to_string(total_bike_loop_count+1), COLOR_WHITE);
            env.console.log("Bike Loop " + std::to_string(total_bike_loop_count+1));
            bool hatch_detected = run_bike_loop(env, context);
            if (hatch_detected){
                phase = EggAutoPhase::HATCHING;
                continue;
            }

            // done Bike loop
            ++num_loops_since_last_fetch_attempt;
            ++total_bike_loop_count;
            if (total_bike_loop_count >= MAX_BIKE_LOOP_COUNT){
                // throw exception
                exceed_bike_loop_limit(env, context, MAX_BIKE_LOOP_COUNT);
            }

            if (num_loops_since_last_fetch_attempt < LOOPS_PER_FETCH){
                phase = EggAutoPhase::BIKE_LOOP; // repeat bike loop
            }else{
                if (m_num_eggs_retrieved < total_eggs_to_fetch){
                    phase = EggAutoPhase::FETCH_EGG;
                }else{
                    // done retrieving eggs
                    // resume hatching eggs if needed
                    phase = EggAutoPhase::BIKE_LOOP;
                }
            }
            continue;
        }
        case EggAutoPhase::HATCHING:{
            env.console.log("Hatching egg.");
            num_eggs_hatched = hatch_routine(env, context, stats, num_eggs_hatched);
            phase = EggAutoPhase::FLY_RESET;
            continue;
        }
        case EggAutoPhase::FLY_RESET:{
            env.console.log("Call flying taxi to reset position.");
            const bool fly_from_overworld = true;
            bool hatch_detected = call_flying_taxi(env, context, fly_from_overworld);
            if (hatch_detected){
                phase = EggAutoPhase::HATCHING;
            }else{
                phase = EggAutoPhase::BIKE_LOOP;
            }
            continue;
        }
        case EggAutoPhase::FETCH_EGG:{
            env.console.log("Talk to lady to fetch egg.");
            EggFetchResult fetch_result = talk_to_lady_to_fetch_egg(env, context, stats);
            if (fetch_result.hatch_detected){
                phase = EggAutoPhase::HATCHING;
            }else{
                if(fetch_result.found_egg){ 
                    m_num_eggs_retrieved++; 
                    env.log("Found egg " + std::to_string(m_num_eggs_retrieved) + "/" + std::to_string(total_eggs_to_fetch), COLOR_WHITE);
                    env.console.overlay().add_log("Found egg " + std::to_string(m_num_eggs_retrieved) + "/" + std::to_string(total_eggs_to_fetch), COLOR_WHITE);
                    stats.m_fetch_success++;
                    env.update_stats();
                }

                // if spoke to lady, back to the bike loop regardless of the success of the egg fetch
                num_loops_since_last_fetch_attempt = 0;
                phase = EggAutoPhase::BIKE_LOOP;
            }
            continue;
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
    m_num_eggs_in_party_at_batch_start = 5;
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
        save_game(env.console, context);
        m_num_eggs_in_storage_when_game_saved = 0;
        m_num_eggs_in_party_when_game_saved = 5;
    }
    
    context.wait_for_all_requests();
    return false;
}

bool EggAutonomous::run_bike_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    YCommIconWatcher no_overworld(COLOR_RED, false);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            travel_to_spin_location(context);
            travel_back_to_lady(context);
        },
        {{no_overworld}}
    );

    if (ret == 0){ // no overworld detected. Check if we find egg hatching.
        BlackDialogBoxWatcher2 egg_hatching_detector;
        int ret2 = wait_until(
            env.console, context,
            std::chrono::seconds(10),
            {
                egg_hatching_detector,
            }
        );
        if (ret2 == 0){
            env.console.log("Hatching detected during bike loop.");
            return true;
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_bike_loop: No recognized state after 10 seconds.",
                env.console
            );
        }
    }

    return false;
}

void EggAutonomous::exceed_bike_loop_limit(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    size_t max_bike_loop_count
){
    env.console.log("Reached max number of bike loops " + std::to_string(max_bike_loop_count));
    env.console.overlay().add_log("Error: max loops " + std::to_string(max_bike_loop_count), COLOR_WHITE);
    env.console.log("Take a screenshot of party to debug.");
    // Now take a photo at the player's party for dumping debug info:
    // Enter Rotom Phone menu
    menus_to_mainmenu(env.console, context);
    // Select Pokemon App
    navigate_to_menu_app(env.console, context, POKEMON_APP_INDEX);
    // From menu enter Pokemon App
    ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, EGG_BUTTON_HOLD_DELAY);
    context.wait_for_all_requests();

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Max number of loops reached. Not enough eggs in party?",
        env.console
    );

}

size_t EggAutonomous::hatch_routine(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats,
    size_t num_eggs_hatched
){
    // confirm we are starting with the hatching screen
    BlackDialogBoxWatcher2 egg_hatching_detector;
    int ret0 = wait_until(
        env.console, context,
        std::chrono::seconds(5),
        {
            egg_hatching_detector,
        }
    );
    if (ret0 < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "hatch_routine: We expected to see a hatching egg, but no hatching detected.",
            env.console
        );
    }

    int ret = -1;
    do{
        ++num_eggs_hatched;
        stats.m_hatched++;
        env.update_stats();
        wait_for_egg_hatched(env, context, stats, num_eggs_hatched);

        // Now we see if we can hatch one more egg.
        ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                //  Try move a little to hatch more:
                //  We move toward lower-left so that it wont hit the lady or enter the Nursory.
                //  Add 1 second of settle time to stop moving.
                //  We need to not be moving before trying to fly or an egg will hatch during
                //  that sequence when we cannot handle it.
                pbf_move_left_joystick(context, {-1, -1}, 800ms, 1000ms);
            },
            {{egg_hatching_detector}}
        );
    }while (ret == 0);

    return num_eggs_hatched;
}


bool EggAutonomous::call_flying_taxi(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    bool fly_from_overworld
){

    BlackDialogBoxWatcher2 egg_hatching_detector;
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            context.wait_for_all_requests();
            env.console.overlay().add_log("Call Flying Taxi", COLOR_WHITE);
            if (fly_from_overworld){
                // Open menu
                env.log("Fly from overworld to reset position");
                menus_to_mainmenu(env.console, context);
            }else{
                env.log("Fly from menu to reset position");
            }

            navigate_to_menu_app(env.console, context, TOWN_MAP_APP_INDEX);            
        },
        {{egg_hatching_detector}}
    );

    bool hatch_detected = ret == 0;
    if (hatch_detected){
        env.console.log("Hatching detected while trying to call flying taxi.");
    }else {
        fly_home(context, false);
        mash_B_until_y_comm_icon(env.console, context, "Cannot detect end of flying taxi animation.");
    }

    return hatch_detected;
}

void EggAutonomous::wait_for_egg_hatched(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats,
    size_t num_hatched_eggs
){
    env.console.overlay().add_log("Egg hatching " + std::to_string(num_hatched_eggs) + "/5", COLOR_GREEN);
    const bool y_comm_visible_at_end_of_egg_hatching = true;
    YCommIconWatcher end_egg_hatching_detector(COLOR_RED, y_comm_visible_at_end_of_egg_hatching);
    const int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 60000ms);
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

EggFetchResult EggAutonomous::talk_to_lady_to_fetch_egg(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggAutonomous_Descriptor::Stats& stats
){
    env.log("Fetching egg");
    stats.m_fetch_attempts++;
    env.update_stats();

    YCommIconWatcher overworld;
    RetrieveEggArrowFinder egg_arrow_detector(env.console);
    CheckNurseryArrowFinder no_egg_arrow_detector(env.console);
    WhiteDialogBoxWatcher white_dialog;
    BlackDialogBoxWatcher2 black_dialog;

    size_t seen_overworld = 0;
    bool egg_status_known = false;
    bool found_egg = false;

    WallClock deadline = current_time() + std::chrono::minutes(2);
    while(!egg_status_known && current_time() < deadline){
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(30),
            {
                overworld,
                egg_arrow_detector,
                black_dialog,
                white_dialog,
                no_egg_arrow_detector,
            }
        );
        switch (ret){
        case 0: // overworld
            env.log("Detected Overworld...", COLOR_BLUE);
            seen_overworld++;
            if (seen_overworld > 10){
                // No NPC found
                env.log("Stuck in Overworld. Daycare lady not found.", COLOR_BLUE);
                return EggFetchResult{
                    .found_egg = false,
                    .hatch_detected = false
                };
            }
            pbf_press_button(context, BUTTON_A, 160ms, 100ms);
            continue;            
        case 1: // egg_arrow_detector
            env.log("Found egg");            
            found_egg = true;
            // Press A to get the egg
            ssf_press_button(context, BUTTON_A, 320ms, 160ms);
            continue;        
        case 2: // black_dialog
            if (found_egg){
                env.log("Received egg");
                egg_status_known = true;  // break the loop. then mash B

            }else{ // this black dialog might actually be a hatching egg.
                env.log("Hatching detected while trying to talk to lady to fetch egg.");
                return EggFetchResult{
                    .found_egg = false,
                    .hatch_detected = true
                };
            }
            continue;
        case 3: // white_dialog
            env.log("Detected dialog box...", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 40ms);
            continue;            
        case 4: // no_egg_arrow_detector
            env.log("No egg");
            env.console.overlay().add_log("No egg", COLOR_WHITE);
            found_egg = false;
            egg_status_known = true;  // break the loop. then mash B
            continue;     
        default:
           OperationFailedException::fire(
               ErrorReport::SEND_ERROR_REPORT,
               "talk_to_lady_to_fetch_egg(): No recognized state after 30 seconds.",
               env.console
           );
        }
    }

    if (!egg_status_known){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "talk_to_lady_to_fetch_egg(): Unable to speak to lady after 2 minutes.",
            env.console
        );
    }

    // we know if the egg was found or not
    // mash b to return to overworld
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 30s);
        },
        {{overworld}}
    );
    if (ret2 < 0){ // If dialog over is not detected:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Cannot detect end of Nursery lady dialog. No Y-Comm mark found.",
            env.console
        );
    }

    return EggFetchResult{ 
        .found_egg = found_egg,
        .hatch_detected = false
    };
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

    menus_to_boxsystem(env.console, context);

    const Milliseconds BOX_CHANGE_DELAY = GameSettings::instance().BOX_CHANGE_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;

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
            pbf_wait(context, 400ms); // wait for a while to make sure the pokemon stats are loaded.
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
                pbf_press_button(context, BUTTON_A, 160ms, 400ms);
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
                pbf_press_dpad(context, DPAD_UP, 160ms, 160ms);
                pbf_press_dpad(context, DPAD_UP, 160ms, 160ms);

                // Press A to release
                pbf_press_button(context, BUTTON_A, 160ms, 840ms);
                // Move cursor from "Not release" to "release".
                pbf_press_dpad(context, DPAD_UP, 160ms, 240ms);
                // Press A to confirm release, wait for a while to let the next dialog box pop up.
                pbf_press_button(context, BUTTON_A, 160ms, 1600ms);

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
                pbf_press_button(context, BUTTON_A, 160ms, 800ms);
                
                size_t dialog_count = 0;
                const size_t max_dialog_count = 6;
                for (; dialog_count < max_dialog_count; dialog_count++){
                    context.wait_for_all_requests();
                    if (!dialog_detector.process_frame(env.console.video().snapshot(), current_time())){
                        break;
                    }
                    pbf_press_button(context, BUTTON_A, 160ms, 800ms);
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

    //  Back out to menu.
    menus_to_mainmenu(env.console, context);

    if (need_taxi){
        bool fly_from_overworld = false; // fly from menu
        call_flying_taxi(env, context, fly_from_overworld);
    }else{
        // Leave menu, go back to overworld
        const bool y_comm_visible = true;
        YCommIconWatcher y_comm_detector(COLOR_RED, y_comm_visible);
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





}
}
}

