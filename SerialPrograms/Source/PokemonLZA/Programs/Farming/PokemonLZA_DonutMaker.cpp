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
#include "PokemonLZA/Programs/PokemonLZA_FastTravelNavigation.h"
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
        , kept(m_stats["Donuts Kept"])
        , resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        // TODO: Add more stats here
        m_display_order.emplace_back("Donuts Matched");
        m_display_order.emplace_back("Donuts Kept");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& matched;
    std::atomic<uint64_t>& kept;
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
    , MAX_KEEPERS(
        "<b>Maximum Number of Donuts to Keep:</b><br>"
        "The program will stop when this many donuts are kept or all limits in the table are reached, whichever happens first."
        "<br>Make sure you have enough berries to make this many donuts. The program will fail when not given enough berries.",
        LockMode::LOCK_WHILE_RUNNING,
        5, 1, 999
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
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(FLAVOR_POWERS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

// Split flavor power notation into its individual tokens
std::vector<std::string> get_flavor_power_tokens(const std::string& power_string){
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = power_string.find('-');
    while (end != std::string::npos){
        tokens.push_back(power_string.substr(start, end - start));
        start = end + 1;
        end = power_string.find('-', start);
    }
    tokens.push_back(power_string.substr(start));
    return tokens;
}

// Check if all flavor tokens are in the donut flavor tokens
// Apply some extra logic for "any" token and the special level tokens
bool flavor_tokens_are_subset(const std::vector<std::string>& subset, const std::vector<std::string>& superset){
    for (const std::string& token : subset){
        if (token == "any"){
            continue;
        }
        else if (token == "12"){
            if (std::find(superset.begin(), superset.end(), "1") == superset.end() && std::find(superset.begin(), superset.end(), "2") == superset.end()){
                return false;
            }
        }
        else if (token == "23"){
            if (std::find(superset.begin(), superset.end(), "2") == superset.end() && std::find(superset.begin(), superset.end(), "3") == superset.end()){
                return false;
            }
        }
        else if (std::find(superset.begin(), superset.end(), token) == superset.end()){
            return false;
        }
    }
    return true;
}

// Check if a donut matches an individual table entry. Each table entry has three target powers.
bool donut_matches_powers(
    SingleSwitchProgramEnvironment& env,
    std::vector<std::string>& donut_powers,
    const std::vector<std::string>& target_powers
){
    for (const std::string& target_power : target_powers){
        // `target_power` can be a specific power like "item-power-berries-3" or a slug to match
        // any possible power (or no power): "any-power-any".
        std::vector<std::string> target_tokens = get_flavor_power_tokens(target_power);
        bool matched = false;
        for (const std::string& donut_power : donut_powers){
            env.log("Comparing target power " + target_power + " with donut power " + donut_power);
            std::vector<std::string> donut_tokens = get_flavor_power_tokens(donut_power);
            if (flavor_tokens_are_subset(target_tokens, donut_tokens)){
                env.log("Power matched!");
                matched = true;
                break;
            }
        }
        if (!matched){
            return false;
        }
    }
    return true;
}

// Read flavor power and check if they match user requirement.
// Keep or discard the donut depending on the user defined limit.
// Return true if the user requirement is fulfilled.
bool DonutMaker::match_powers(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<uint16_t>& kept_counts
){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Reading in table of desired powers.");

    std::vector<std::vector<std::string>> powers_table;
    std::vector<std::unique_ptr<FlavorPowerTableRow>> wanted_powers_table = FLAVOR_POWERS.copy_snapshot();
    for (const std::unique_ptr<FlavorPowerTableRow>& row : wanted_powers_table){
        FlavorPowerTableEntry table_line = row->snapshot();
        powers_table.push_back(table_line.get_entry_flavor_power_strings());
        env.log(table_line.to_str());
    }
    //TODO: Validate powers? The "All Types" type only applies to catching and sparkling powers. Move and resist do not have "All Types"
    //Are people even going to target move and resist power? Big Haul/Item Berry/Alpha/Sparkling seems more likely.

    env.log("Checking donut powers against table.");
    VideoSnapshot screen = env.console.video().snapshot();
    std::vector<std::string> donut_results;
    for (int i = 0; i < 3; i++){
        FlavorPowerDetector read_power(env.logger(), COLOR_GREEN, LANGUAGE, i);
        const std::string power = read_power.detect_power(screen);
        if (power.empty()){
            env.add_overlay_log(std::format("{}: Empty", i+1));
            continue;
        }
        env.add_overlay_log(std::format("{}: {}", i+1, power));
        donut_results.push_back(power);
    }

    bool match_found = false;
    bool should_keep = false;
    // Each power table row:
    for (size_t i = 0; i < powers_table.size(); i++){
        if (donut_matches_powers(env, donut_results, powers_table[i])){
            match_found = true;
            if (kept_counts[i] < FLAVOR_POWERS.snapshot()[i].limit){
                kept_counts[i]++;
                env.log(
                    "Keeping donut: " + std::to_string(kept_counts[i]) + " / " +
                    std::to_string(FLAVOR_POWERS.snapshot()[i].limit) +
                    " for table entry " + std::to_string(i+1)
                );
                should_keep = true;
            }
        }
    }
    if (match_found){
        env.log("Match found!");
        stats.matched++;
        env.update_stats();
        if (should_keep){
            stats.kept++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_DONUT_FOUND, "Match found! Keeping donut.", screen, true);
            return true;
        } else {
            env.log("Matched donut exceeds all keep limits.");
        }
    }
    env.log("Discarding donut.");
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

        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(30),
            {
                white_dialog,
                arrow,
                berry_selection,
                dreams_alpha_delta,
                omega,
            }
        );
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
    open_fast_travel_menu(env.console, context);
    set_fast_travel_menu_filter(env.console, context, FAST_TRAVEL_FILTER::ALL_TRAVEL_SPOTS);

    // Close out of map
    exit_menu_to_overworld(env, context);
    context.wait_for_all_requests();

    // The filters should now be set to "Facilities" and hovering over Centrico Plaza
    env.log("Fast travel map filters reset.");
}

// Move to in front of Ansha with button A shown
void DonutMaker::move_to_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    FastTravelState travel_status = open_map_and_fly_to(env.console, context, LANGUAGE, Location::HOTEL_Z);
    if (travel_status != FastTravelState::SUCCESS){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Cannot fast travel to Hotel Z.",
            env.console
        );
    }
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
    if (duration >= 10s){
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
void DonutMaker::save_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Creating new backup save to keep the last made donut.");

    // Stop talking to Ansha
    exit_menu_to_overworld(env, context);
    context.wait_for_all_requests();
}

// Check if all user defined limits are reached or the global max keepers limit is reached
bool DonutMaker::should_stop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const std::vector<uint16_t>& kept_counts,
    uint16_t total_kept
){
    bool limit_reached = true;
    for (size_t i = 0; i < kept_counts.size(); i++){
        if (kept_counts[i] < FLAVOR_POWERS.snapshot()[i].limit){
            limit_reached = false;
        }
    }
    if (total_kept >= MAX_KEEPERS){
        return true;
    }
    return limit_reached;
}

// Return true if a donut match is found
bool DonutMaker::donut_iteration(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<uint16_t>& kept_counts
){
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

    // Read flavor power and check if they match user requirement and should be kept:
    if (match_powers(env, context, kept_counts)){
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

    std::vector<uint16_t> kept_counts(FLAVOR_POWERS.snapshot().size(), 0);
    uint16_t total_kept = 0;
    while(true){
        const bool should_keep = donut_iteration(env, context, kept_counts);
        stats.resets++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);

        if (should_keep){
            total_kept++;
            if (should_stop(env, context, kept_counts, total_kept)){
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
