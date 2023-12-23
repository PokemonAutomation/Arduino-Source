/*  Flying Trail Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_FlyingTrailFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


FlyingTrailFarmer_Descriptor::FlyingTrailFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:FlyingTrailFarmer",
        STRING_POKEMON + " SV", "Flying Trail Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FlyingTrailFarmer.md",
        "Farm the flying trail for BP.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct FlyingTrailFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_trials(m_stats["Trialss"])
    {
        m_display_order.emplace_back("Trials");
    }
    std::atomic<uint64_t>& m_trials;
};
std::unique_ptr<StatsTracker> FlyingTrailFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



FlyingTrailFarmer::FlyingTrailFarmer()
    : GO_HOME_WHEN_DONE(false)
    , NUM_TRIALS(
        "<b>Number of Trials to Run (10BP/Trial):</b>",
        LockMode::LOCK_WHILE_RUNNING,
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



void FlyingTrailFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    FlyingTrailFarmer_Descriptor::Stats& stats = env.current_stats<FlyingTrailFarmer_Descriptor::Stats>();

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
            env.log("Black screen detected. Trail starting.");
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
            pbf_move_left_joystick(context, 200,  30, 1 * TICKS_PER_SECOND, 0); // go through the 2nd ring
            pbf_wait(context,  2 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context,  40,  50, 2 * TICKS_PER_SECOND, 0);
            pbf_wait(context,  1 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128,  50, 2 * TICKS_PER_SECOND, 0); // adjust vertical height
            pbf_wait(context,  6 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 115, 128, 1 * TICKS_PER_SECOND, 0); // horizontal angle adjustment, might overshoot in rare cases
            pbf_wait(context,  7 * TICKS_PER_SECOND);
            pbf_move_left_joystick(context, 128, 180, 2 * TICKS_PER_SECOND, 0);
            pbf_wait(context,  780);
            pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 0);
        }

        OverworldWatcher overworld(COLOR_CYAN);
        context.wait_for_all_requests();

        int ret_finish = run_until(
            env.console, context,
            [](BotBaseContext& context) {
                for (int i = 0; i < 1000; i++){
                    pbf_mash_button(context, BUTTON_B, 10000);
                }
            },
            { overworld }
        );
        context.wait_for_all_requests();
        if (ret_finish == 0) {
            env.log("Overworld detected. Trail completed.");
        }
        // TODO: Detect success/fail cases based on either the black dialog box or BP number on top right

        stats.m_rounds++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
