/*  Friendship Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_FriendshipFarmer.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/ScreenshotException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

FriendshipFarmer_Descriptor::FriendshipFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:FriendshipFarmer",
        Pokemon::STRING_POKEMON + " LZA", "Friendship Farmer",
        "Programs/PokemonLZA/FriendshipFarmer.html",
        "Farm friendship for pokemon in your party using either the cafe or bench method.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{
}

class FriendshipFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : drinks_shared(m_stats["Drinks Shared"])
        , hang_out_sessions(m_stats["Hang Out Sessions"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Drinks Shared");
        m_display_order.emplace_back("Hang Out Sessions");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& drinks_shared;
    std::atomic<uint64_t>& hang_out_sessions;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> FriendshipFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

FriendshipFarmer::~FriendshipFarmer(){
    FARMING_OPTION.remove_listener(*this);
}

FriendshipFarmer::FriendshipFarmer()
    : FARMING_OPTION(
        "<b>Farming Method</b>",
        {
            { FarmingOption::Cafe, "Cafe", "Share drinks at a Cafe" },
            { FarmingOption::Bench, "Bench", "Hangout on a bench" }
        },
        LockMode::LOCK_WHILE_RUNNING,
        FarmingOption::Cafe
    )
    , FRIENDSHIP_AMOUNT(
        "<b>Friendship Amount</b>",
        {
            { FriendshipAmount::Evolve, "Evolve", "Increase friendship until pokemon can evolve." },
            { FriendshipAmount::Max, "Max", "Increase friendship to maximum." }
        },
        LockMode::LOCK_WHILE_RUNNING,
        FriendshipAmount::Evolve
    )
    , NUM_PARTY_MEMBERS(
        "<b>Number of Party Members:</b><br><i>Only used for Cafe Method</i>",
        LockMode::LOCK_WHILE_RUNNING,
        6, 1, 6
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_OPTION("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_OPTION,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(FARMING_OPTION);
    PA_ADD_OPTION(FRIENDSHIP_AMOUNT);
    PA_ADD_OPTION(NUM_PARTY_MEMBERS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    FriendshipFarmer::on_config_value_changed(this);

    FARMING_OPTION.add_listener(*this);
}

void FriendshipFarmer::on_config_value_changed(void* object){
    if (FARMING_OPTION == FarmingOption::Cafe){
        NUM_PARTY_MEMBERS.set_visibility(ConfigOptionState::ENABLED);
    }
    else {
        NUM_PARTY_MEMBERS.set_visibility(ConfigOptionState::HIDDEN);
    }
}

void FriendshipFarmer::enter_cafe(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    FriendshipFarmer_Descriptor::Stats& stats = env.current_stats<FriendshipFarmer_Descriptor::Stats>();

    bool seen_selection_arrow = false;

    while (true)
    {
        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            { 0.1, 0.1, 0.8, 0.8 },
            &env.console.overlay()
        );
        ButtonWatcher buttonB_watcher(
            COLOR_WHITE,
            ButtonType::ButtonB,
            { 0.911, 0.939, 0.019, 0.033 },
            &env.console.overlay()
        );
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            { 0.6300, 0.153, 0.2260, 0.627 }
        );
        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay());
        BlueDialogWatcher blue_dialog_watcher(COLOR_BLUE, &env.console.overlay());
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA_watcher,
                buttonB_watcher,
                selection_arrow_watcher,
                white_dialog_watcher,
                blue_dialog_watcher,
                black_screen
            }
        );
        context.wait_for(100ms);

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
        int ret2 = 0;

        switch (ret){
        case 0:
            env.log("Detected A button.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 1:
            env.log("Detected B button.");
            pbf_press_button(context, BUTTON_B, 80ms, 40ms);
            return;
        case 2:
            env.log("Detected selection arrow.");
            seen_selection_arrow = true;
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 3:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 4:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 5:
            if (seen_selection_arrow){
                env.log("Detected loading screen. Entering cafe.");
                continue;
            }

            env.log("Detected day change.");
            ret2 = wait_until(
                env.console, context,
                30s,
                { overworld }
            );

            if (ret2 == 0){
                env.log("Returned to overworld.");
                continue;
            }

            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_cafe(): Unable to detect overworld after day/night change.",
                env.console
            );
        default:
            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_cafe(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

void FriendshipFarmer::exit_bench(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    FriendshipFarmer_Descriptor::Stats& stats = env.current_stats<FriendshipFarmer_Descriptor::Stats>();

    int reset_attempt = 0;
    bool seen_selection_arrow = false;

    while (true)
    {
        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            { 0.1, 0.1, 0.8, 0.8 },
            &env.console.overlay()
        );
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            { 0.6300, 0.4440, 0.2260, 0.3190 }
        );
        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay());
        BlueDialogWatcher blue_dialog_watcher(COLOR_RED, &env.console.overlay());
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA_watcher,
                selection_arrow_watcher,
                white_dialog_watcher,
                blue_dialog_watcher,
                black_screen
            }
        );
        context.wait_for(100ms);

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
        int ret2 = 0;

        switch (ret){
        case 0:
            env.log("Detected A button.");
            return;
        case 1:
            env.log("Detected selection arrow.");
            seen_selection_arrow = true;
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 3:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 4:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 5:
            env.log("Detected day change.");
            ret2 = wait_until(
                env.console, context,
                30s,
                {overworld}
            );

            if (ret2 == 0){
                env.log("Returned to overworld.");
                continue;
            }

            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "exit_bench(): Unable to detect overworld after day/night change.",
                env.console
            );
        default:
            if (seen_selection_arrow && ++reset_attempt <= 10){
                env.log("Attempting to face the bench.");
                pbf_move_left_joystick(context, {0, +1}, 250ms, 0ms);
                continue;
            }

            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "exit_bench(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

void FriendshipFarmer::exit_cafe(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    FriendshipFarmer_Descriptor::Stats& stats = env.current_stats<FriendshipFarmer_Descriptor::Stats>();

    while (true){
        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            { 0.1, 0.1, 0.8, 0.8 },
            &env.console.overlay()
        );
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            { 0.6300, 0.4440, 0.2260, 0.3190 }
        );
        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay());
        BlueDialogWatcher blue_dialog_watcher(COLOR_RED, &env.console.overlay());
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA_watcher,
                selection_arrow_watcher,
                white_dialog_watcher,
                blue_dialog_watcher,
                black_screen
            }
        );
        context.wait_for(100ms);

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
        int ret2 = 0;

        switch (ret){
        case 0:
            env.log("Detected A button.");
            return;
        case 1:
            env.log("Detected selection arrow.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 3:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 4:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 5:
            env.log("Detected loading screen while leaving cafe caused by day/night change. Waiting..");
            ret2 = wait_until(
                env.console, context,
                30s,
                { overworld }
            );

            if (ret2 == 0){
                env.log("Returned to overworld.");
                continue;
            }

            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "exit_cafe(): Unable to detect overworld after day/night change.",
                env.console
            );
        default:
            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "exit_cafe(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

void FriendshipFarmer::hang_out_bench(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    FriendshipFarmer_Descriptor::Stats& stats = env.current_stats<FriendshipFarmer_Descriptor::Stats>();

    bool seen_selection_arrow = false;
    while (true){
        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            { 0.1, 0.1, 0.8, 0.8 },
            &env.console.overlay()
        );
        ButtonWatcher buttonB_watcher(
            COLOR_WHITE,
            ButtonType::ButtonB,
            { 0.902, 0.937, 0.022, 0.038 },
            &env.console.overlay()
        );
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            { 0.6300, 0.4440, 0.2260, 0.3190 }
        );
        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay());
        BlueDialogWatcher blue_dialog_watcher(COLOR_BLUE, &env.console.overlay());
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA_watcher,
                buttonB_watcher,
                selection_arrow_watcher,
                white_dialog_watcher,
                blue_dialog_watcher,
                black_screen
            }
        );
        context.wait_for(100ms);

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
        int ret2 = 0;

        switch (ret){
        case 0:
            env.log("Detected A button.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 1:
            env.log("Detected B button. Waiting 10 Seconds."); //Wait 10 seconds to gain friendship points.
            context.wait_for(10000ms);
            pbf_press_button(context, BUTTON_B, 80ms, 40ms);
            return;
        case 2:
            env.log("Detected selection arrow.");
            seen_selection_arrow = true;
            //Select second option to hang out on bench.
            pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 3:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 4:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 5:
            if (seen_selection_arrow){
                env.log("Detected loading screen. Hanging out on bench.");
                continue;
            }

            env.log("Detected day change.");
            ret2 = wait_until(
                env.console, context,
                30s,
                { overworld }
            );

            if (ret2 == 0){
                env.log("Returned to overworld.");
                continue;
            }

            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "hang_out_bench(): Unable to detect overworld after day/night change.",
                env.console
            );
        default:
            ++stats.errors;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "hang_out_bench(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

void FriendshipFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    FriendshipFarmer_Descriptor::Stats& stats = env.current_stats<FriendshipFarmer_Descriptor::Stats>();

    if (FARMING_OPTION.get() == FarmingOption::Cafe){
        env.log("Cafe Method Started");

        for (int i = 0; i < NUM_PARTY_MEMBERS; ++i){
            int drinks_needed;

            //Each drink at cafe gives 30 friendship points.
            //Evolve = 160 points = 5.33 drinks
            //Max = 255 points = 8.5 drinks
            if (FRIENDSHIP_AMOUNT.get() == FriendshipAmount::Evolve){
                drinks_needed = 6;
            }
            else {
                drinks_needed = 9;
            }

            for (int d = 0; d < drinks_needed; ++d){
                enter_cafe(env, context);
                exit_cafe(env, context);

                ++stats.drinks_shared;
                env.update_stats();
            }

            env.log("Switching to next Party Member");
            pbf_press_dpad(context, DPAD_RIGHT, 40ms, 40ms);
            context.wait_for(100ms);
        }

        env.log("Cafe Method Finished");
    }
    else{
        env.log("Bench Method Started");
        int hang_outs_needed;

        //Each hangout on bench gives 10 friendship points.
        // Evolve = 160 points = 16 hangouts
        // Max = 255 points = 26 hangouts
        if (FRIENDSHIP_AMOUNT.get() == FriendshipAmount::Evolve){
            hang_outs_needed = 16;
        }
        else{
            hang_outs_needed = 26;
        }

        for (int i = 0; i < hang_outs_needed; ++i)
        {
            hang_out_bench(env, context);
            exit_bench(env, context);

            ++stats.hang_out_sessions;
            env.update_stats();
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

}
}
}
