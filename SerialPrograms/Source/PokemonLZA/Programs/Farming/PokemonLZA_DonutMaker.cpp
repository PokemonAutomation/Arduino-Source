/*  Donut Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutBerriesDetector.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_FlavorPowerDetector.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_FlavorPowerScreenDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_DonutBerrySession.h"
#include "PokemonLZA_DonutMaker.h"
#include <format>

// After pressing + to send the berries to Ansha:
// - A white button dialog window, pressing A to clear
// - An animation of Ansha making donut with Hoopa. End with her showing the donut. pressing A during
//   the animatino clears the animation. Press A to end the fully played animation
// - Ansha claps while the donut presenatation animation plays. Flavor power shows after a coupld of seconds


namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


DonutMaker_Descriptor::DonutMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:DonutMaker", STRING_POKEMON + " LZA",
        "Donut Maker",
        "Programs/PokemonLZA/DonutMaker.html",
        "Make donuts and reset until desired flavor powers are found.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class DonutMaker_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : matched(m_stats["Donuts Matched"])
        , resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        // TODO: Add more stats here
        m_display_order.emplace_back("Donuts Matched");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& matched;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> DonutMaker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


DonutMaker::DonutMaker()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , BERRIES("<b>Berries:</b><br>The berries used to make the donut. Minimum 3 berries, maximum 8 berries.")
    , NUM_POWER_REQUIRED(
        "<b>Number of Powers to Match:</b><br>How many of a donut's powers must be in the the table below. Minimum 1, maximum 3. "
        "<br>Ex. For a target donut of Big Haul Lv.3, Berry Lv.3, and any or none for the 3rd power, set the number as 2."
        "<br>Then, in the flavor powers table, make sure to add Big Haul Lv.3 and Berry Lv. 3.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 3
        )
    , NUM_DONUTS(
       "<b>Number of Donuts:</b><br>The number of donuts to make."
       "<br>Make sure you have enough berries to make this many donuts.",
       LockMode::LOCK_WHILE_RUNNING,
       1, 1, 999
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_DONUT_FOUND(
        "Donut Found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_DONUT_FOUND,
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BERRIES);
    PA_ADD_OPTION(NUM_POWER_REQUIRED);
    PA_ADD_OPTION(NUM_DONUTS);
    PA_ADD_OPTION(FLAVOR_POWERS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

// Read flavor power and check if they match user requirement.
// Return true if the user requirement is fulfilled.
bool DonutMaker::match_powers(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Reading in table of desired powers.");
    std::vector<std::string> power_table;
    std::vector<std::unique_ptr<FlavorPowerTableRow>> wanted_powers_table = FLAVOR_POWERS.copy_snapshot();
    for (const std::unique_ptr<FlavorPowerTableRow>& row : wanted_powers_table){
        FlavorPowerTableEntry table_line = row->snapshot();
        power_table.push_back(table_line.to_str());
        env.log(table_line.to_str());
    }
    //TODO: Validate powers? The "All Types" type only applies to catching and sparkling powers. Move and resist do not have "All Types"
    //Are people even going to target move and resist power? Big Haul/Item Berry/Alpha/Sparkling seems more likely.

    env.log("Reading powers and counting up hits.");
    uint8_t num_hits = 0;
    VideoSnapshot screen = env.console.video().snapshot();
    for (int i = 0; i < 3; i++) {
        FlavorPowerDetector read_power(env.logger(), COLOR_GREEN, LANGUAGE, i);
        const std::string power = read_power.detect_power(screen);
        if (power.empty()){
            env.add_overlay_log(std::format("{}: Empty", i+1));
            continue;
        }
        env.add_overlay_log(std::format("{}: {}", i+1, power));
        
        if ((std::find(power_table.begin(), power_table.end(), power) != power_table.end())) {
            num_hits++;
        }
    }

    if (num_hits >= NUM_POWER_REQUIRED) {
        stats.matched++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_DONUT_FOUND, "Match found!", screen, true);
        return true;
    }

    return false;
}

void DonutMaker::animation_to_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

#if 0
    WhiteScreenOverWatcher animation_skip(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, Seconds(3));
            pbf_wait(context, Seconds(30));
        },
        {animation_skip}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
           ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to skip donut making animation.",
            env.console
        );
    }

    //Press A and wait a bit for powers to appear
    pbf_wait(context, Seconds(3));
    pbf_press_button(context, BUTTON_A, 100ms, 200ms);
    pbf_wait(context, Seconds(3));
    context.wait_for_all_requests();
#else
    // Safe initial mashing to clear dialogs
    pbf_mash_button(context, BUTTON_A, Seconds(3));
    context.wait_for_all_requests();

    env.add_overlay_log("Waiting for Flavor Power Screen...");

    FlavorPowerScreenWatcher flavor_power_screen_watcher;
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for(int i = 0; i < 20; i++){
                pbf_press_button(context, BUTTON_A, 100ms, 400ms);
            }
        },
        {{flavor_power_screen_watcher}}
    );
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find the donut flavor power screen.",
            env.console
        );
    }
    env.log("Found donut flavor power screen");
    env.add_overlay_log("Detected Flavor Power Screen");

    // Wait for the UI animation for revealing flavor powers to play out
    pbf_wait(context, Seconds(3));
    context.wait_for_all_requests();
    env.add_overlay_log("Read Flavor Powers");
#endif
}

void DonutMaker::add_berries_and_make_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Checking berry count.");
    std::vector<std::unique_ptr<DonutBerriesTableRow>> berries_table = BERRIES.copy_snapshot();
    env.log("Number of berries validated.", COLOR_BLACK);

    //Berries map for selection
    std::map<std::string, uint8_t> processed_berries;
    for (const std::unique_ptr<DonutBerriesTableRow>& row : berries_table){
        const std::string& table_item = row->berry.slug();
        processed_berries[table_item]++;
    }

    add_donut_berries(env.console, context, LANGUAGE, std::move(processed_berries));
    animation_to_donut(env, context);
}

// Press A to talk to Ansha and keep pressing A until reach the berry selection menu
void DonutMaker::open_berry_menu_from_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    // press button A to start talking to Ansha
    pbf_press_button(context, BUTTON_A, 100ms, 200ms);
    context.wait_for_all_requests();
    
    WallClock start = current_time();
    while(current_time() - start <= Seconds(120)){
        FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &env.console.overlay());
        SelectionArrowWatcher arrow(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.591, 0.579, 0.231, 0.105}
        );
        SelectionArrowWatcher dreams_alpha_delta(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.591, 0.517, 0.231, 0.105}
        );
        SelectionArrowWatcher omega(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.591, 0.450, 0.231, 0.105}
        );
        DonutBerriesSelectionWatcher berry_selection(0);

        int ret = wait_until(env.console, context, std::chrono::seconds(3),
            {white_dialog, arrow, berry_selection, dreams_alpha_delta, omega});
        switch (ret){
        case 0:
            env.log("Detected white dialog. Go to next dialog");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 1:
            env.log("Detected selection arrow. Go to next dialog");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 2:
            env.log("Berry selection menu shown.");
            env.add_overlay_log("Found Berry Selection Menu");
            return;
        case 3: case 4:
            env.log("Detected selection arrow for Bad Dreams/Alpha/Omega/Delta recipe.");
            pbf_press_dpad(context, DPAD_DOWN, 100ms, 200ms);
            break;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "donut_maker(): Unable to detect white dialog, selection arrow or berry menu after talking to Ansha.",
                env.console
            );
        } // end switch(ret)
        context.wait_for_all_requests();
    } // end while(true)

    stats.errors++;
    env.update_stats();
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "donut_maker(): 2 minutes passed yet unable to reach berry menu after taking to Ansha.",
        env.console
    );
}

// A generic function to fast travel to an index in the fast travel menu and watch for overworld
void fast_travel_to_index(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int location_index=0){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Fast traveling to location at index " + std::to_string(location_index));
    bool zoom_to_max = false;
    const bool require_icons = false;
    open_map(env.console, context, zoom_to_max, require_icons);
    
    // Press Y to load fast travel locaiton menu
    pbf_press_button(context, BUTTON_Y, 100ms, 500ms);
    context.wait_for_all_requests();

    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            // Move cursor to desired location index
            for (int i = 0; i < location_index; i++){
                pbf_press_dpad(context, DPAD_DOWN, 50ms, 500ms);
            }
            pbf_mash_button(context, BUTTON_A, Seconds(10));
            pbf_wait(context, Seconds(30)); // 30 sec to wait out potential day night change
        },
        {overworld}
    );
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
           ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find overworld after fast traveling to location index " + std::to_string(location_index),
            env.console
        );
    }
    env.log("Detected overworld. Fast traveled to location index " + std::to_string(location_index));
}

// Exit the game and load the backup save
void load_backup_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Making backup save.");

    go_home(env.console, context);
    const bool backup_save = true;
    if (!reset_game_from_home(env, env.console, context, backup_save)){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Cannot reset game from Switch Home screen.",
            env.console
        );
    }
}

void exit_menu_to_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Exiting menu to overworld.");

    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, Seconds(10));
            pbf_wait(context, Seconds(30)); // 30 sec to wait out potential day night change
        },
        {overworld}
    );
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
           ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find overworld after exiting menu.",
            env.console
        );
    }
}

void reset_map_filter_state(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Resetting fast travel map filters.");

    open_map(env.console, context, false, false);
    // Press Y and - to open fast travel filter menu
    pbf_press_button(context, BUTTON_Y, 100ms, 500ms);
    pbf_press_button(context, BUTTON_MINUS, 100ms, 500ms);
    // Press Down and A to select "Facilities" filter
    pbf_press_dpad(context, DPAD_DOWN, 100ms, 500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);

    // Close out of map
    exit_menu_to_overworld(env, context);
    context.wait_for_all_requests();

    // The filters should now be set to "Facilities" and hovering over Centrico Plaza
    env.log("Fast travel map filters reset.");
}

// Move to in front of Ansha with button A shown
void move_to_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    fast_travel_to_index(env, context, 3); // Fast travel to Hotel Z
    context.wait_for(100ms); // Wait for player control to return
    env.log("Detected overworld. Fast traveled to Hotel Zone");

    int ret = run_towards_gate_with_A_button(env.console, context, 0, +1, Seconds(5));
    if (ret == 1){  // day night change happens during running
        // As day night change has ended, try running towards door again
        if (run_towards_gate_with_A_button(env.console, context, 0, +1, Seconds(5)) != 0){
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "donut_maker(): Cannot reach Hotel Z gate after day/night change.",
                env.console
            );
            // overworld.last_detected_frame()->save("debug_overworld_detection.png");
        }
    } else if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Cannot reach Hotel Z gate after fast travel.",
            env.console
        );
        // overworld.last_detected_frame()->save("debug_overworld_detection.png");
    }

    // Mash button A to enter the hotel.
    pbf_mash_button(context, BUTTON_A, Seconds(2));
    context.wait_for_all_requests();
    WallClock start_time = current_time();
    // We use 50s here to account for day night change
    wait_until_overworld(env.console, context, 50s);
    env.console.log("Detected overworld after entering zone.");
    WallClock end_time = current_time();
    const auto duration = end_time - start_time;
    // Due to day/night change may eating the mashing button A sequence, we may still be outside the hotel!
    if (duration >= 16s){
        // mash A again to make sure we are inside the hotel
        pbf_mash_button(context, BUTTON_A, Seconds(2));
        context.wait_for_all_requests();
        wait_until_overworld(env.console, context, 50s);
    }

    
    // we are now inside the hotel

    // Roll forward twice
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    pbf_move_left_joystick(context, {-1, 0}, 500ms, 100ms);
    context.wait_for_all_requests();
}

// Create a new backup save after making a donut to keep
void save_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Creating new backup save to keep the last made donut.");

    // Stop talking to Ansha
    exit_menu_to_overworld(env, context);
    context.wait_for_all_requests();

    // Fast travel to anywhere to set a new backup save after making a donut to keep
    // Removed this since it's likely redundant because the program always fast travels to Hotel Z before making a donut
    // fast_travel_to_index(env, context, 0, 3000ms);
}

// Return true if it should stop
bool DonutMaker::donut_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    move_to_ansha(env, context);

    const ImageFloatBox button_A_box{0.3, 0.2, 0.4, 0.7};
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, button_A_box, &env.console.overlay());
    int ret = wait_until(env.console, context, std::chrono::seconds(3), {buttonA});
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find button A facing Ansha.",
            env.console
        );
    }

    open_berry_menu_from_ansha(env, context);
    // Add berries from menu and make a donut. Stop at flavor power screen.
    add_berries_and_make_donut(env, context);

    // Read flavor power and check if they match user requirement:
    if (match_powers(env, context)){
        return true;
    }

    load_backup_save(env, context);
    return false;
}


void DonutMaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    assert_16_9_1080p_min(env.logger(), env.console);

    if (LANGUAGE == Language::None){
        throw UserSetupError(env.console, "Must set game language option to read berries and flavor powers.");
    }

    {
        const std::vector<std::unique_ptr<DonutBerriesTableRow>> berries_table = BERRIES.copy_snapshot();
        const size_t num_berries = berries_table.size();
        if (num_berries < 3 || num_berries > 8) {
            throw UserSetupError(env.console, "Must have at least 3 berries and no more than 8 berries.");
        }
    }

    //  Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 200ms);

    reset_map_filter_state(env, context);
    while(true){
        const bool should_stop = donut_iteration(env, context);
        stats.resets++;
        env.update_stats();

        if (should_stop){
            if (stats.matched.load() >= NUM_DONUTS){
                break;
            }
            save_donut(env, context);
        }
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
