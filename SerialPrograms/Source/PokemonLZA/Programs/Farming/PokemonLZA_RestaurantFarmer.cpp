/*  Restaurant Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonTools/Async/InterruptableCommands.h"
#include "CommonTools/Async/InferenceRoutines.h"
//#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_RunFromBattleDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_TrainerBattle.h"
#include "PokemonLZA_RestaurantFarmer.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


RestaurantFarmer_Descriptor::RestaurantFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:RestaurantFarmer",
        STRING_POKEMON + " LZA", "Restaurant Farmer",
        "Programs/PokemonLZA/RestaurantFarmer.html",
        "Farm the restaurants for exp, items, and money.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class RestaurantFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : rounds(m_stats["Rounds"])
        , wins(m_stats["Wins"])
        , losses(m_stats["Losses"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Battles"] = "Rounds";
    }

    std::atomic<uint64_t>& rounds;
    std::atomic<uint64_t>& wins;
    std::atomic<uint64_t>& losses;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> RestaurantFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


RestaurantFarmer::RestaurantFarmer()
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


bool RestaurantFarmer::run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

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
        FlatWhiteDialogWatcher dialog0(COLOR_RED, &env.console.overlay());
        BlueDialogWatcher dialog1(COLOR_RED, &env.console.overlay());
        ItemReceiveWatcher item_receive(COLOR_RED, &env.console.overlay());
        RunFromBattleWatcher battle_menu(COLOR_GREEN, &env.console.overlay(), 250ms);

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA, // 0
                arrow,   // 1
                dialog0, // 2
                dialog1, // 3 
                item_receive, // 4
                battle_menu,  // 5
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("Detected A button.");
            if (STOP_AFTER_CURRENT.should_stop()){
                return true;
            }
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 1:
            env.log("Detected selection arrow.");
            // This is when the restaurant receptionist is asking whether you want
            // to start the round
            // Mash A for 5 sec to clear rest of the dialog and enter battle
            pbf_mash_button(context, BUTTON_A, 5000ms);
            return false;

        case 2:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 3:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 4:
            env.log("Detected item receive.");
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 5:
            env.log("Detected battle menu. (unexpected)", COLOR_RED);
            stats.errors++;
            return false;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_lobby(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}
void RestaurantFarmer::run_round(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

//    WallClock start = current_time();

    bool won = false;
    TrainerBattleState battle_state(BATTLE_AI);

    while (true){
        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.4, 0.3, 0.2, 0.7},
            &env.console.overlay(),
            1000ms
        );
        SelectionArrowWatcher arrow(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.654308, 0.481553, 0.295529, 0.312621},
            1000ms
        );
        ItemReceiveWatcher item_receive(COLOR_RED, &env.console.overlay(), 1000ms);
        FlatWhiteDialogWatcher dialog0(COLOR_RED, &env.console.overlay(), 1000ms);
        BlueDialogWatcher dialog1(COLOR_RED, &env.console.overlay(), 1000ms);
        RunFromBattleWatcher battle_menu(COLOR_GREEN, &env.console.overlay(), 10ms);

        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120s);
            },
            {
                buttonA,      // 0
                arrow,        // 1
                item_receive, // 2
                dialog0,      // 3
                dialog1,      // 4
                battle_menu,  // 5
            }
        );

        switch (ret){
        case 0:
            env.log("Detected Lobby.");
            stats.rounds++;
            if (won){
                stats.wins++;
            }else{
                stats.losses++;
            }
            env.update_stats();
            return;

        case 1:
            env.log("Detected selection arrow. (unexpected)", COLOR_RED);
            dump_image(env.console.logger(), env.program_info(), env.console.video(), "UnexpectedSelectionArrow");
            stats.errors++;
            env.update_stats();
            continue;

        case 2:
            env.log("Detected item receive.");
            won = true;
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 3:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 4:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 5:
            env.log("Detected battle menu.");
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


void RestaurantFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);
    pbf_mash_button(context, BUTTON_B, 1000ms);

//    auto lobby = env.console.video().snapshot();

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
