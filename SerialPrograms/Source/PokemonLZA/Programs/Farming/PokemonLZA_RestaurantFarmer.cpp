/*  Restaurant Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "CommonTools/Async/InferenceRoutines.h"
//#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
//#include "PokemonLZA/Inference/PokemonLZA_MoveEffectivenessSymbol.h"
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
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
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


RestaurantFarmer::~RestaurantFarmer(){
    STOP_AFTER_CURRENT.remove_listener(*this);
}

RestaurantFarmer::RestaurantFarmer()
    : m_stop_after_current(false)
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
    , MOVE_AI(
        "<b>Move Selection AI:</b><br>"
        "If enabled, it will be smarter with move selection.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , USE_PLUS_MOVES(
        "<b>Use Plus Moves:</b><br>"
        "If enabled, it will attempt to use plus moves.<br>"
        "However, this adds a 320ms delay which may cause opponent attacks to land first.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(MOVE_AI);
    PA_ADD_OPTION(USE_PLUS_MOVES);

    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(PERIODIC_SAVE);

    PA_ADD_OPTION(NOTIFICATIONS);

    STOP_AFTER_CURRENT.set_idle();
    STOP_AFTER_CURRENT.add_listener(*this);
}



RestaurantFarmer::StopButton::StopButton()
    : ButtonOption(
      "<b>Stop after current round:",
      "Stop after current round",
      0, 16
    )
{}
void RestaurantFarmer::StopButton::set_idle(){
    this->set_enabled(false);
    this->set_text("Stop after Current Round");
}
void RestaurantFarmer::StopButton::set_ready(){
    this->set_enabled(true);
    this->set_text("Stop after Current Round");
}
void RestaurantFarmer::StopButton::set_pressed(){
    this->set_enabled(false);
    this->set_text("Program will stop after current round...");
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

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA,
                arrow,
                dialog0,
                dialog1,
                item_receive,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("Detected A button.");
            if (m_stop_after_current.load(std::memory_order_relaxed)){
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

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_lobby(): No recognized state after 60 seconds.",
                env.console
            );
        }
    }
}
void RestaurantFarmer::run_round(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

    WallClock start = current_time();

    bool won = false;

    while (true){
        context.wait_for_all_requests();

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


        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                while (current_time() - start < 30min){
                    attempt_one_attack(env, context, MOVE_AI, USE_PLUS_MOVES);
                }
            },
            {
                buttonA,
                arrow,
                item_receive,
                dialog0,
                dialog1,
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

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Round took longer than 30 minutes.",
                env.console
            );
        }

    }
}


class RestaurantFarmer::ResetOnExit{
public:
    ResetOnExit(StopButton& button)
        : m_button(button)
    {}
    ~ResetOnExit(){
        m_button.set_idle();
    }

private:
    StopButton& m_button;
};

void RestaurantFarmer::on_press(){
    global_logger_tagged().log("Stop after current requested...");
    m_stop_after_current.store(true, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_pressed();
}

void RestaurantFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();
    m_stop_after_current.store(false, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_ready();
    ResetOnExit reset_button_on_exit(STOP_AFTER_CURRENT);
    pbf_mash_button(context, BUTTON_B, 1000ms);

//    auto lobby = env.console.video().snapshot();

    for (uint32_t rounds_since_last_save = 0;; rounds_since_last_save++){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_ROUNDS != 0 && stats.rounds >= NUM_ROUNDS) {
            m_stop_after_current.store(true, std::memory_order_relaxed);
            STOP_AFTER_CURRENT.set_pressed();
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
