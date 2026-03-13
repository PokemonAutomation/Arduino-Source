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
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_NuggetBridgeFarmer.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"

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
        100, // About 2 hours of farming. 
        0
    )
    , GO_HOME_WHEN_DONE(false)
    , PERIODIC_SAVE(
        "<b>Periodically Save:</b><br>"
        "Save the game every this many nuggets. This reduces the loss to game crashes. Set to zero to disable.",
        LockMode::UNLOCK_WHILE_RUNNING,
        50,
        0
    )
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
    PA_ADD_OPTION(PERIODIC_SAVE);

    PA_ADD_OPTION(NOTIFICATIONS);
}

void NuggetBridgeFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    home_black_border_check(env.console, context);

    NuggetBridgeFarmer_Descriptor::Stats& stats = env.current_stats<NuggetBridgeFarmer_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    for (uint32_t nuggets_since_last_save = 0;; nuggets_since_last_save++){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_NUGGETS != 0 && stats.nuggets >= NUM_NUGGETS){
            break;
        }

        if (PERIODIC_SAVE != 0 && nuggets_since_last_save >= PERIODIC_SAVE){
            env.console.log("Saving game...");

            save_game_to_overworld(env.console, context);
            nuggets_since_last_save = 0;
        }

        env.console.log("Exiting Pokemon Center...");
        while (true){
            BlackScreenWatcher pokemon_ceter_exit(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_press_dpad(context, DPAD_DOWN, 2000ms, 0ms);
                },
                { pokemon_ceter_exit }
            );

            if (ret == 0){
                break;
            }
        }

        env.console.log("Detecting overworld...");
        while (true){
            BlackScreenOverWatcher overworld_entered(COLOR_RED);

            int ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(2000),
                {overworld_entered}
            );

            if (ret == 0){
                break;
            }
        }

        // There is a small delay from seeing the overworld to being able to a actually move.
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();

        env.console.log("Navigating to Team Rocket member...");

        ssf_press_button(context, BUTTON_B, 0ms, 12000ms);
        pbf_press_dpad(context, DPAD_LEFT, 590ms, 0ms);
        pbf_press_dpad(context, DPAD_UP, 510ms, 0ms);
        pbf_press_dpad(context, DPAD_LEFT, 1550ms, 0ms);
        pbf_press_dpad(context, DPAD_UP, 1175ms, 0ms);
        pbf_press_dpad(context, DPAD_RIGHT, 1950ms, 0ms);
        pbf_press_dpad(context, DPAD_UP, 5300ms, 0ms);

        context.wait_for_all_requests();

        env.console.log("Starting battle...");
        while (true){
            BattleMenuWatcher battle_menu(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                },
                { battle_menu }
            );

            if (ret == 0){
                break;
            }
        }

        env.console.log("Loosing battle...");
        while (true){
            BlackScreenWatcher battle_lost(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 2000ms);
                },
                { battle_lost }
            );

            if (ret == 0){
                break;
            }
        }

        env.console.log("Talking to nurse joy...");
        while (true){
            WhiteDialogWatcher nurse_joy_dialog(COLOR_RED);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                },
                { nurse_joy_dialog }
            );

            if (ret == 0){
                break;
            }
        }

        // Spam B till nurse joy stops talking
        while (true){
            WhiteDialogWatcher nurse_joy_dialog(COLOR_RED);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                },
                { nurse_joy_dialog }
            );

            if (ret != 0){
                break;
            }
        }

        stats.nuggets++;
        env.update_stats();

        if (STOP_AFTER_CURRENT.should_stop()){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
