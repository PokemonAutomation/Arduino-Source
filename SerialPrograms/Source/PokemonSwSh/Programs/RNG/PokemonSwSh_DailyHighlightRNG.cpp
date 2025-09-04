/*  RNG Manipulation of the Highlight Watt Trader in the Snowslide Slope area in the Crown Tundra
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Based on Anubis' findings: https://docs.google.com/spreadsheets/u/0/d/1pNYtCJKRh_efX9LvzjCiA-0n2lGSFnVmSWwmPzgSOMw
 *
 */

#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_MenuNavigation.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_DailyHighlightRNG.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


const DailyHighlightDatabase& DAILY_HIGHLIGHT_DATABASE() {
    static DailyHighlightDatabase database("PokemonSwSh/DailyHighlights.json");
    return database;
}


DailyHighlightRNG_Descriptor::DailyHighlightRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DailyHighlightRNG",
        STRING_POKEMON + " SwSh", "Daily Highlight RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DailyHighlightRNG.md",
        "Perform RNG manipulation to get rare items from the daily highlight trader.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct DailyHighlightRNG_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : iterations(m_stats["Iterations"])
        , reads(m_stats["Seed Reads"])
        , errors(m_stats["Errors"])
        , highlights(m_stats["Highlights"])
    {
        m_display_order.emplace_back("Iterations");
        m_display_order.emplace_back("Seed Reads");
        m_display_order.emplace_back("Highlights");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

public:
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& reads;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& highlights;
};
std::unique_ptr<StatsTracker> DailyHighlightRNG_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

DailyHighlightRNG::DailyHighlightRNG()
    : NUM_HIGHLIGHTS(
        "<b>Number of Highlights:</b><br>How many daily highlights should be bought. <br>Zero will run until you stop the program.",
        LockMode::UNLOCK_WHILE_RUNNING, 0)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.<br>Doesn't do anything if Number of Highlights is 0.",
        LockMode::UNLOCK_WHILE_RUNNING, false)
    , GO_HOME_WHEN_DONE(false)
    , SAVE_INTERVAL(
        "<b>Save Every this Many Day Skips:</b><br>Zero disables saving.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20
    )
    , CALIBRATION_INTERAVAL(
        "<b>Calibrate the Number of NPCs this Many Day Skips:</b><br>Zero will only calibrate once.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10
    )
    , HIGHLIGHT_SELECTION(
        "<b>Desired Highlights:</b>", 
        "Highlight", 
        DAILY_HIGHLIGHT_DATABASE().database(), 
        "dream-ball")
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MAX_EXPECTED_NPCS(
        "<b>Max Expected NPCs:</b><br>How many NPCs are expected in the area at most.",
        LockMode::LOCK_WHILE_RUNNING,
        8
    )
    , CALIBRATION_REPEATS(
        "<b>Calibration Repeats:</b><br>How many times the number of NPCs is checked per calibration.",
        LockMode::LOCK_WHILE_RUNNING,
        5
    )
    , CALIBRATION_THRESHOLD(
        "<b>Calibration Treshold:</b><br>How many NPC counts need to be the same.<br>This needs to be less than or equal to Calibration Repeats.",
        LockMode::LOCK_WHILE_RUNNING,
        3
    )
    , MAX_UNKNOWN_ADVANCES(
        "<b>Max Unknown Advances:</b><br>How many advances to check when updating the rng state.",
        LockMode::LOCK_WHILE_RUNNING,
        100000
    )
    , ADVANCE_PRESS_DURATION(
        "<b>Advance Press Duration:</b><br>Hold the button down for this long to advance once.",
        LockMode::LOCK_WHILE_RUNNING,
        "80 ms"
    )
    , ADVANCE_RELEASE_DURATION(
        "<b>Advance Release Duration:</b><br>After releasing the button, wait this long before pressing it again.",
        LockMode::LOCK_WHILE_RUNNING,
        "80 ms"
    )
    , SAVE_SCREENSHOTS(
        "<b>Save Debug Screenshots:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , LOG_VALUES(
        "<b>Log Animation Values:</br>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);

    PA_ADD_OPTION(NUM_HIGHLIGHTS);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(SAVE_INTERVAL);
    PA_ADD_OPTION(CALIBRATION_INTERAVAL);
    PA_ADD_OPTION(HIGHLIGHT_SELECTION);

    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CALIBRATION_REPEATS);
    PA_ADD_OPTION(CALIBRATION_THRESHOLD);
    PA_ADD_OPTION(MAX_EXPECTED_NPCS);
    PA_ADD_OPTION(MAX_UNKNOWN_ADVANCES);
    PA_ADD_OPTION(ADVANCE_PRESS_DURATION);
    PA_ADD_OPTION(ADVANCE_RELEASE_DURATION);
    PA_ADD_OPTION(SAVE_SCREENSHOTS);
    PA_ADD_OPTION(LOG_VALUES);
}

void DailyHighlightRNG::move_to_trader(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    pbf_move_right_joystick(context, 255, 128, 460ms, 80ms);
    pbf_move_left_joystick(context, 128, 0, 1280ms, 80ms);
}

void DailyHighlightRNG::interact_with_trader(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    VideoOverlaySet boxes(env.console);
    SelectionArrowFinder arrow_detector(env.console, ImageFloatBox(0.5, 0.58, 0.2, 0.08));
    arrow_detector.make_overlays(boxes);
    YCommIconDetector y_comm_icon_detector(true);
    size_t tries = 0;

    while (true) {
        if (tries >= 5) {
            DailyHighlightRNG_Descriptor::Stats& stats = env.current_stats<DailyHighlightRNG_Descriptor::Stats>();
            stats.errors++;
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to talk to the trader.",
                env.console
            );
        }
        context.wait_for_all_requests();
        int ret = wait_until(env.console, context, 500ms, { y_comm_icon_detector, arrow_detector });
        if (ret == -1 || ret == 0) {
            tries++;
            pbf_press_button(context, BUTTON_A, 80ms, 80ms);
        }
        else if (ret == 1) {
            return;
        }
    }
}

void DailyHighlightRNG::buy_highlight(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    // The player is expected to be in main dialog of the trader
    env.log("Buying Highlight.");
    env.console.overlay().add_log("Buying Highlight!", COLOR_WHITE);
    pbf_press_dpad(context, DPAD_DOWN, 160ms, 160ms);

    pbf_press_button(context, BUTTON_A, 160ms, 160ms);
    context.wait_for_all_requests();

    VideoOverlaySet boxes(env.console);
    SelectionArrowFinder arrow_detector(env.console, ImageFloatBox(0.5, 0.58, 0.2, 0.08));
    arrow_detector.make_overlays(boxes);

    int ret = run_until<ProControllerContext>(
        env.console,
        context, 
        [](ProControllerContext& context) { 
            pbf_mash_button(context, BUTTON_A, 20000ms);
        },
        { {arrow_detector} }
    );

    if (ret < 0) {
        DailyHighlightRNG_Descriptor::Stats& stats = env.current_stats<DailyHighlightRNG_Descriptor::Stats>();
        stats.errors++;
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Could not detect dialog.",
            env.console
        );
    }
}

void DailyHighlightRNG::navigate_to_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_wait(context, 200ms);
    navigate_to_menu_app(env, env.console, context, 1, NOTIFICATION_ERROR_FATAL);
    pbf_press_button(context, BUTTON_A, 80ms, 3000ms);
}

void DailyHighlightRNG::save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    env.log("Saving.");
    env.console.overlay().add_log("Saving!", COLOR_WHITE);
    pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_wait(context, 200ms);
    pbf_press_button(context, BUTTON_R, 80ms, 2000ms);
    pbf_mash_button(context, BUTTON_A, 500ms);

    return_to_overworld(env, context);
}

uint8_t DailyHighlightRNG::calibrate_num_npc_from_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Pokemon::Xoroshiro128Plus& rng) {
    env.log("Calibrating NPC count.");
    env.console.overlay().add_log("Calibrating NPC count.", COLOR_WHITE);

    DailyHighlightRNG_Descriptor::Stats& stats = env.current_stats<DailyHighlightRNG_Descriptor::Stats>();
    size_t max_expected_npcs = MAX_EXPECTED_NPCS;
    size_t calibration_repeats = CALIBRATION_REPEATS;
    size_t calibration_threshold = CALIBRATION_THRESHOLD;
    size_t finished_calibration_steps = 0;
    std::vector<size_t> npc_counts(max_expected_npcs, 0);
    while (finished_calibration_steps < calibration_repeats) {
        env.log("Calibration: " + std::to_string(finished_calibration_steps+1) + "/" + std::to_string(calibration_repeats));
        env.console.overlay().add_log("Calibration: " + std::to_string(finished_calibration_steps+1) + "/" + std::to_string(calibration_repeats), COLOR_WHITE);

        return_to_overworld(env, context);
        navigate_to_party(env, context);

        Xoroshiro128PlusState state_before_menu_close(rng.get_state());
        std::pair<Xoroshiro128PlusState, uint64_t> result = refind_rng_state_and_animations(env.console, context, rng.get_state(), 0, 100, SAVE_SCREENSHOTS, LOG_VALUES);
        Xoroshiro128PlusState new_state = result.first;
        uint64_t additional_advances = result.second;

        // Calculate state for possible NPC counts
        std::vector<Xoroshiro128PlusState> rng_states;

        for (uint8_t npcs = 0; npcs <= max_expected_npcs; npcs++) {
            Xoroshiro128PlusState temp_state = predict_state_after_menu_close(state_before_menu_close, npcs);
            Xoroshiro128Plus temp_rng(temp_state);

            // Do advances that were needed to refind current state
            for (size_t i = 0; i < additional_advances; i++) {
                temp_rng.next();
            }
            rng_states.push_back(temp_rng.get_state());
        }

        // Compare current state to expected states
        uint8_t num_npcs = 0;
        uint8_t possible_choices = 0;
        for (uint8_t i = 0; i <= max_expected_npcs; i++) {
            if (rng_states[i].s0 == new_state.s0 && rng_states[i].s1 == new_state.s1) {
                num_npcs = i;
                possible_choices++;
            }
        }
        rng.state = new_state;

        stats.reads++;

        // Add another calibration round if multiple npcs counts are possible
        if (possible_choices > 1) {
            env.log("NPC count is ambiguous.");
            env.console.overlay().add_log("NPC count is ambiguous. Repeat calibration.", COLOR_WHITE);
            continue;
        }

        env.console.log("Calculated there are " + std::to_string(num_npcs) + " NPCs.");
        env.console.overlay().add_log(std::to_string(num_npcs) + " NPCs", COLOR_WHITE);
        npc_counts[num_npcs]++;
        finished_calibration_steps++;

        if (npc_counts[num_npcs] >= calibration_threshold) {
            env.console.log("Final Calibration: " + std::to_string(num_npcs) + " NPCs.");
            env.console.overlay().add_log("Final Calibration: " + std::to_string(num_npcs) + " NPCs", COLOR_WHITE);

            return num_npcs;
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "NPC is in the wrong state or an unexpected number of NPCs is in the area.",
        env.console
    );
}

size_t DailyHighlightRNG::calculate_target(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, uint8_t num_npcs, std::vector<std::string> wanted_highlights) {
    Xoroshiro128Plus rng(state);
    size_t advances = 0;
    bool found_advance_amount = false;

    std::vector<std::pair<uint16_t, uint16_t>> ranges; 
    for (std::string slug : wanted_highlights) {
        ranges.push_back(DAILY_HIGHLIGHT_DATABASE().get_range_for_slug(slug));
    }

    while (!found_advance_amount) {
        // Calculate the result for the current temp_rng state

        Xoroshiro128PlusState temp_state = predict_state_after_menu_close(rng.get_state(), num_npcs);
        Xoroshiro128Plus temp_rng(temp_state);

        uint64_t highlight_roll = temp_rng.nextInt(1000);

        for (auto& range : ranges) {
            if (range.first <= highlight_roll && range.second >= highlight_roll) {
                found_advance_amount = true;
                env.console.log("Target highlight roll: " + std::to_string(highlight_roll));
            }
        }
        
        if (!found_advance_amount) {
            rng.next();
            advances++;
        }
    }

    return advances;
}

void DailyHighlightRNG::prepare_game_state(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    env.log("Prepare player position.");
    env.console.overlay().add_log("Reset Player Position.", COLOR_WHITE);
    
    // Open map
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_wait(context, 200ms);
    navigate_to_menu_app(env, env.console, context, 5, NOTIFICATION_ERROR_RECOVERABLE);
    pbf_wait(context, 200ms);
    pbf_press_button(context, BUTTON_A, 160ms, 4000ms);

    // Fly to Snowslide Slope
    pbf_move_left_joystick(context, 200, 210, 160ms, 160ms);
    pbf_move_left_joystick(context, 150, 165, 160ms, 160ms);
    pbf_mash_button(context, BUTTON_A, 1500ms);
    return_to_overworld(env, context);
}

void DailyHighlightRNG::return_to_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool wait_after_detection) {
    context.wait_for_all_requests();
    env.console.log("Returning to the overworld.");
    YCommIconDetector y_comm_icon_detector(true);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_B, 20000ms);
        },
        { {y_comm_icon_detector} }
    );
    if (ret != 0) {
        DailyHighlightRNG_Descriptor::Stats& stats = env.current_stats<DailyHighlightRNG_Descriptor::Stats>();
        stats.errors++;
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Cannot detect the Y-Comm icon.",
            env.console
        );
    }

    // Sometimes Y-Comm is detected before button presses are accepted again
    if (wait_after_detection) {
        context.wait_for(300ms); 
    }
}

void DailyHighlightRNG::advance_date(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t& year) {
    context.wait_for(200ms);
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
    home_to_date_time(env.console, context, true);
    pbf_press_button(context, BUTTON_A, 160ms, 240ms);
    context.wait_for_all_requests();

    VideoOverlaySet overlays(env.console.overlay());
    DateChangeWatcher date_reader(env.console);
    date_reader.make_overlays(overlays);

    DateTime date{ 2000, 12, 31, 1, 0, 0 }; // 31st December for fixed Normal weather
    
    if (year >= MAX_YEAR) {
        date_reader.set_date(env.program_info(), env.console, context, date);
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        year = 1;
    }

    date.year += year;
    date_reader.set_date(env.program_info(), env.console, context, date);
    pbf_press_button(context, BUTTON_A, 160ms, 240ms);

    pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    resume_game_from_home(env.console, context, false);
    year++;
}


void DailyHighlightRNG::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DailyHighlightRNG_Descriptor::Stats& stats = env.current_stats<DailyHighlightRNG_Descriptor::Stats>();
    env.update_stats();

    std::vector<std::string> wanted_highlights = HIGHLIGHT_SELECTION.all_slugs();
    if (wanted_highlights.empty()){
        throw UserSetupError(env.console, "At least one highlight item needs to be selected!");
    }

    if (START_LOCATION.start_in_grip_menu()) {
        grip_menu_connect_go_home(context);
    }
    else {
        pbf_press_button(context, BUTTON_B, 80ms, 80ms);
    }


    Xoroshiro128Plus rng(0, 0);
    uint8_t num_npcs = 0;
    size_t iterations = 0;
    size_t assumed_successful_iterations = 0;
    size_t bought_highlights = 0;
    uint8_t year = MAX_YEAR;
    uint16_t state_errors = 0;

    while (bought_highlights < NUM_HIGHLIGHTS || NUM_HIGHLIGHTS <= 0){
        iterations++;
        stats.iterations++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        env.console.log("Daily Highlight RNG iteration: " + std::to_string(iterations));
        env.console.overlay().add_log("Iteration: " + std::to_string(iterations), COLOR_WHITE);

        if (SAVE_INTERVAL > 0 && iterations % SAVE_INTERVAL == 0) {
            save_game(env, context);
        }

        return_to_overworld(env, context);

        if (assumed_successful_iterations <= 0) {
            prepare_game_state(env, context);

            // open and close the menu and immediately walk to the trader
            pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
            return_to_overworld(env, context);
            move_to_trader(env, context);
            interact_with_trader(env, context);

            return_to_overworld(env, context);
        }

        advance_date(env, context, year);
        navigate_to_party(env, context);
        context.wait_for_all_requests();

        // (Re-)Find RNG state
        if (assumed_successful_iterations <= 0) {
            env.log("Finding initial rng state.");
            env.console.overlay().add_log("Initial RNG state:", COLOR_WHITE);
            rng = Xoroshiro128Plus(find_rng_state(env.console, context, SAVE_SCREENSHOTS, LOG_VALUES));
            stats.reads++;
        } else {
            env.log("Refinding rng state.");
            env.console.overlay().add_log("New RNG state:", COLOR_WHITE);
            rng = Xoroshiro128Plus(refind_rng_state(env.console, context, rng.get_state(), 0, MAX_UNKNOWN_ADVANCES, SAVE_SCREENSHOTS, LOG_VALUES));
            stats.reads++;
        }

        Xoroshiro128PlusState rng_state = rng.get_state();
        if (rng_state.s0 == 0 && rng_state.s1 == 0){
            stats.errors++;
            env.update_stats();

            state_errors++;
            if (state_errors >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Detected invalid RNG state three times in a row.",
                    env.console
                );
            }
            VideoSnapshot screen = env.console.video().snapshot();
            send_program_recoverable_error_notification(env, NOTIFICATION_ERROR_RECOVERABLE, "Detected invalid RNG state.", screen);
            assumed_successful_iterations = 0;
            continue;
        }

        // Calibrate number of NPCs in the area and check whether the trader is in the slow state
        if ((CALIBRATION_INTERAVAL > 0 && assumed_successful_iterations % CALIBRATION_INTERAVAL == 0) || assumed_successful_iterations <= 0) {
            try {
                num_npcs = calibrate_num_npc_from_party(env, context, rng);
            }
            catch (OperationFailedException& exception) {
                send_program_recoverable_error_notification(
                    env,
                    NOTIFICATION_ERROR_RECOVERABLE,
                    exception.message(),
                    exception.screenshot_view()
                );
                assumed_successful_iterations = 0;
                stats.errors++;
                continue;
            }
        }

        // Do required advances
        size_t target_advances = calculate_target(env, rng.get_state(), num_npcs, HIGHLIGHT_SELECTION.all_slugs());
        env.console.log("Needed advances: " + std::to_string(target_advances));
        do_rng_advances(env.console, context, rng, target_advances, ADVANCE_PRESS_DURATION, ADVANCE_RELEASE_DURATION);

        // Talk to NPC and buy highlight
        return_to_overworld(env, context, false);
        interact_with_trader(env, context);
        if (assumed_successful_iterations >= 2) {
            buy_highlight(env, context);
            bought_highlights++;
            stats.highlights++;
        }
        return_to_overworld(env, context);
        
        env.update_stats();
        assumed_successful_iterations++;
        state_errors = 0;
    }

    if (FIX_TIME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH, "Finished buying " + std::to_string(bought_highlights) + " daily highlights!", env.console.video().snapshot());
}


}
}
}
