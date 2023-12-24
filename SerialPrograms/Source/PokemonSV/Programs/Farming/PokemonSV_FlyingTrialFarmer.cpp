/*  Flying Trial Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV_FlyingTrialFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


FlyingTrialFarmer_Descriptor::FlyingTrialFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:FlyingTrialFarmer",
        STRING_POKEMON + " SV", "Flying Trial Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FlyingTrialFarmer.md",
        "Farm the flying trial for BP.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct FlyingTrialFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_trials(m_stats["Trials"])
        , m_success(m_stats["Success"])
        , m_fail(m_stats["Fail"])
    {
        m_display_order.emplace_back("Trials");
        m_display_order.emplace_back("Success");
        m_display_order.emplace_back("Fail");
    }
    std::atomic<uint64_t>& m_trials;
    std::atomic<uint64_t>& m_success;
    std::atomic<uint64_t>& m_fail;
};
std::unique_ptr<StatsTracker> FlyingTrialFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



FlyingTrialFarmer::FlyingTrialFarmer()
    : GO_HOME_WHEN_DONE(false)
    , NUM_TRIALS(
        "<b>Number of Trials to Run (10BP/Trial):</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        20
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NUM_TRIALS);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool FlyingTrialFarmer::run_rewards(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    bool trial_failed = true;
    while (true){
        DialogBoxWatcher dialog(COLOR_GREEN, true, std::chrono::milliseconds(250), DialogType::DIALOG_BLACK);
        OverworldWatcher overworld(COLOR_CYAN);

        context.wait_for_all_requests();

        int ret_finish = run_until(
            env.console, context,
            [](BotBaseContext& context) {
                pbf_mash_button(context, BUTTON_B, 10000);
            },
            { dialog, overworld }
        );

        switch (ret_finish){
        case 0: // dialog
            trial_failed = false;
            continue;
        case 1: // overworld
            return trial_failed;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "No recognized state after 80 seconds.",
                true
            );
        }
    }
}

void FlyingTrialFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    FlyingTrialFarmer_Descriptor::Stats& stats = env.current_stats<FlyingTrialFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 0);

    for (uint8_t i = 0; i < NUM_TRIALS; i++){
        BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
        context.wait_for_all_requests();

        int ret_entry = run_until(
            env.console, context,
            [](BotBaseContext& context) {
                pbf_mash_button(context, BUTTON_A, 10000);
            },
            { black_screen }
        );
        context.wait_for_all_requests();
        if (ret_entry == 0) {
            env.log("Black screen detected. Trial starting.");
        }

        WhiteButtonWatcher whitebutton(COLOR_GREEN, WhiteButton::ButtonY, {0.45, 0.85, 0.10, 0.14});
        context.wait_for_all_requests();
        int ret_trial_start = wait_until(
            env.console, context,
            std::chrono::seconds(120),
            {whitebutton}
        );
        if (ret_trial_start == 0) {
            env.log("Countdown is over. Start navigation sequence.");
            pbf_wait(context,  3 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 180,  20, 1 * TICKS_PER_SECOND, 0); // go through the 2nd ring
            pbf_wait(context,  2 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context,  40,  50, 240, 0);
            pbf_wait(context,  1 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128,  50, 2 * TICKS_PER_SECOND, 0); // adjust vertical height
            pbf_wait(context, 13 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 180, 2 * TICKS_PER_SECOND, 0);
            pbf_wait(context,  9 * TICKS_PER_SECOND);
        }

        if (!run_rewards(env, context)){
            stats.m_success++;
        } else {
            stats.m_fail++;
        }

        stats.m_trials++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
