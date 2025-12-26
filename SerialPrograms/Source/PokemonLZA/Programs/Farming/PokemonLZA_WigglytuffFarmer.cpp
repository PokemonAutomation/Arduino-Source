/*  Wigglytuff Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_RunFromBattleDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_TrainerBattle.h"
#include "PokemonLZA_WigglytuffFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


WigglytuffFarmer_Descriptor::WigglytuffFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:WigglytuffFarmer",
        STRING_POKEMON + " LZA", "Wigglytuff Farmer",
        "Programs/PokemonLZA/WigglytuffFarmer.html",
        "Farm the Side Mission 157 (DLC) for money.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class WigglytuffFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : rounds(m_stats["Rounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_aliases["Battles"] = "Rounds";
    }

    std::atomic<uint64_t>& rounds;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> WigglytuffFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


WigglytuffFarmer::WigglytuffFarmer()
    : STOP_AFTER_CURRENT("Round")
    , NUM_ROUNDS(
        "<b>Number of Rounds to Run:</b><br>"
        "Zero will run until 'Stop after Current Round' is pressed or the program is manually stopped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        500,
        0
    )
    , GO_HOME_WHEN_DONE(false)
    , PERIODIC_SAVE(
        "<b>Periodically Save:</b><br>"
        "Save the game every this many rounds. This reduces the loss to game crashes. Set to zero to disable.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10,
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
    PA_ADD_OPTION(BATTLE_AI);

    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(PERIODIC_SAVE);

    PA_ADD_OPTION(NOTIFICATIONS);
}


bool WigglytuffFarmer::run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WigglytuffFarmer_Descriptor::Stats& stats = env.current_stats<WigglytuffFarmer_Descriptor::Stats>();

    while (true){
        context.wait_for_all_requests();

        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.4, 0.3, 0.2, 0.7},
            &env.console.overlay()
        );
        SelectionArrowWatcher arrow(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.654308, 0.481553, 0.295529, 0.312621}
        );
        FlatWhiteDialogWatcher dialog(COLOR_RED, &env.console.overlay());
        RunFromBattleWatcher battle_menu(COLOR_GREEN, &env.console.overlay(), 250ms);

        int ret = wait_until(
            env.console, context,
            30000ms,
            {
                buttonA, // 0
                arrow,   // 1
                dialog, // 2
                battle_menu,  // 3
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("lobby: Detected A button.");
            if (STOP_AFTER_CURRENT.should_stop()){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 1:
            env.log("lobby: Detected selection arrow.");
            // This is when the rich boy asking whether you want
            // to start the round
            // Mash A for 5 sec to clear rest of the dialog and enter battle
            pbf_mash_button(context, BUTTON_A, 5000ms);
            return false;

        case 2:
            env.log("lobby: Detected white dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 3:
            env.log("lobby: Detected battle menu. (unexpected)", COLOR_RED);
            stats.errors++;
            return false;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_lobby(): No recognized state after 30 seconds.",
                env.console
            );
        }
    }
}
void WigglytuffFarmer::run_round(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WigglytuffFarmer_Descriptor::Stats& stats = env.current_stats<WigglytuffFarmer_Descriptor::Stats>();

    TrainerBattleState battle_state(BATTLE_AI);

    while (true){
        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.4, 0.3, 0.2, 0.7},
            &env.console.overlay(),
            1000ms
        );
        RunFromBattleWatcher battle_menu(COLOR_GREEN, &env.console.overlay(), 10ms);

        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120s);
            },
            {
                buttonA,      // 0
                battle_menu,  // 1
            }
        );

        switch (ret){
        case 0:
            env.log("battle: Detected Lobby.");
            stats.rounds++;
            env.update_stats();
            return;

        case 1:
            env.log("battle: Detected battle menu.");
            battle_state.attempt_one_attack(env, env.console, context);
            continue;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_round(): No state detected for 2 minutes.",
                env.console
            );
        }

    }
}


void WigglytuffFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    WigglytuffFarmer_Descriptor::Stats& stats = env.current_stats<WigglytuffFarmer_Descriptor::Stats>();

    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);
    pbf_mash_button(context, BUTTON_B, 1000ms);

    for (uint32_t rounds_since_last_save = 0;; rounds_since_last_save++){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_ROUNDS != 0 && stats.rounds >= NUM_ROUNDS) {
            break;
        }

        uint32_t periodic_save = PERIODIC_SAVE;
        if (periodic_save != 0 && rounds_since_last_save >= periodic_save){
            save_game_to_menu(env.console, context);
            pbf_mash_button(context, BUTTON_B, 2000ms);
            rounds_since_last_save = 0;
        }

        if (run_lobby(env, context)){
            break;
        }
        run_round(env, context);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
