/*  Nugget Bridge Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG_NuggetBridgeFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

NuggetBridgeFarmer_Descriptor::NuggetBridgeFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:NuggetBridgeFarmer",
        Pokemon::STRING_POKEMON + " FRLG", "Nugget Bridge Farmer",
        "Programs/PokemonFRLG/NuggetBridgeFarmer.html",
        "Farm the Nugget Bridge for money.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct NuggetBridgeFarmer_Descriptor::Stats : public StatsTracker {
    public:
    Stats()
        : nuggets(m_stats["Nuggets"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Nuggets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& nuggets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> NuggetBridgeFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

NuggetBridgeFarmer::NuggetBridgeFarmer()
    : STOP_AFTER_CURRENT("Nugget")
    , NUM_NUGGETS(
        "<b>Number of Nuggets:</b><br>"
        "Zero will run until 'Stop after Current Nugget' is pressed or the program is manually stopped.",
        LockMode::UNLOCK_WHILE_RUNNING, 
        120, // About 2 hours of farming. 
        0
    )
    , GO_HOME_WHEN_DONE(false)
    /*, PERIODIC_SAVE(
        "<b>Periodically Save:</b><br>"
        "Save the game every this many nuggets. This reduces the loss to game crashes. Set to zero to disable.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10,
        0
    )*/
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);

    PA_ADD_OPTION(NUM_NUGGETS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    //PA_ADD_OPTION(PERIODIC_SAVE);

    PA_ADD_OPTION(NOTIFICATIONS);
}

void NuggetBridgeFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {

    NuggetBridgeFarmer_Descriptor::Stats& stats = env.current_stats<NuggetBridgeFarmer_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    for (uint32_t nuggets_since_last_save = 0;; nuggets_since_last_save++) {
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_NUGGETS != 0 && stats.nuggets >= NUM_NUGGETS) {
            break;
        }

        env.console.log("Exiting Pokemon Center...");
        while (true)
        {
            BlackScreenWatcher pokemon_ceter_exit(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    pbf_press_dpad(context, DPAD_DOWN, 2000ms, 0ms);
                },
                { pokemon_ceter_exit }
            );

            if (ret == 0) {
                break;
            }
        }
        // Wait for the screen to load after exiting. 
        // TODO: Is there something we can watch for instead of just waiting a fixed time?
        pbf_wait(context, 2300ms);
        context.wait_for_all_requests();

        env.console.log("Navigating to Team Rocket member...");

        ssf_press_button(context, BUTTON_B, 0ms, 12000ms);
        ssf_press_dpad(context, DPAD_LEFT, 0ms, 590ms);
        ssf_press_dpad(context, DPAD_UP, 0ms, 510ms);
        ssf_press_dpad(context, DPAD_LEFT, 0ms, 1550ms);
        ssf_press_dpad(context, DPAD_UP, 0ms, 1175ms);
        ssf_press_dpad(context, DPAD_RIGHT, 0ms, 1950ms);
        ssf_press_dpad(context, DPAD_UP, 0ms, 5300ms);

        context.wait_for_all_requests();

        env.console.log("Starting battle...");
        while (true)
        {
            BattleMenuWatcher battle_menu(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                },
                { battle_menu }
            );

            if (ret == 0) {
                break;
            }
        }

        env.console.log("Loosing battle...");
        while (true)
        {
            BlackScreenWatcher battle_lost(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    pbf_mash_button(context, BUTTON_A, 2000ms);
                },
                { battle_lost }
            );

            if (ret == 0) {
                break;
            }
        }

        env.console.log("Talking to nurse joy...");
        while (true)
        {
            WhiteDialogWatcher nurse_joy_dialog(COLOR_RED);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                },
                { nurse_joy_dialog }
            );

            if (ret == 0) {
                break;
            }
        }

        // Spam B till nurse joy stops talking
        while (true)
        {
            WhiteDialogWatcher nurse_joy_dialog(COLOR_RED);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                },
                { nurse_joy_dialog }
            );

            if (ret != 0) {
                break;
            }
        }

        //TODO: Implement periodic saving. The Save Menu keeps the last cursor position. Need to implement arrow dectection on the correct option
        /*if (PERIODIC_SAVE != 0 && nuggets_since_last_save >= PERIODIC_SAVE) {
            StartMenuWatcher start_menu = StartMenuWatcher(COLOR_RED);

            env.console.log("Saving game...");

            while (true)
            {
                int ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context) {
                        pbf_press_button(context, BUTTON_PLUS, 320ms, 640ms);
                        pbf_wait(context, 100ms);
                        context.wait_for_all_requests();
                    },
                    { start_menu }
                );

                if (ret == 0) {
                    break;
                }
            }

            pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);

            pbf_press_button(context, BUTTON_A, 320ms, 320ms);
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);

            pbf_mash_button(context, BUTTON_B, 2000ms);
            nuggets_since_last_save = 0;
        }*/

        stats.nuggets++;
        env.update_stats();

    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}