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
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_MoveEffectivenessSymbol.h"
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
        : battles(m_stats["Battles"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& battles;
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
        "<b>Number of Battles to run:</b><br>Zero will run until 'Stop after Current Battle' is pressed.</b>", 
        LockMode::UNLOCK_WHILE_RUNNING, 
        100, 
        0
        ),
      GO_HOME_WHEN_DONE(false),
      NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600)),
      NOTIFICATIONS({
          &NOTIFICATION_STATUS_UPDATE,
          &NOTIFICATION_PROGRAM_FINISH,
          &NOTIFICATION_ERROR_FATAL,
      })
    , MOVE_AI(
        "<b>Move Selection AI:</b><br>"
        "If enabled, it will be smarter with move selection.<br>"
        "However, this adds a split-second delay which may cause opponent attacks to land first.",
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
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(MOVE_AI);
    PA_ADD_OPTION(USE_PLUS_MOVES);

    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    STOP_AFTER_CURRENT.set_idle();
    STOP_AFTER_CURRENT.add_listener(*this);
}



RestaurantFarmer::StopButton::StopButton()
    : ButtonOption(
      "<b>Stop after current battle:",
      "Stop after current battle",
      0, 16
    )
{}
void RestaurantFarmer::StopButton::set_idle(){
    this->set_enabled(false);
    this->set_text("Stop after Current Battle");
}
void RestaurantFarmer::StopButton::set_ready(){
    this->set_enabled(true);
    this->set_text("Stop after Current Battle");
}
void RestaurantFarmer::StopButton::set_pressed(){
    this->set_enabled(false);
    this->set_text("Program will stop after current battle...");
}



bool RestaurantFarmer::run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

    while (true){
        context.wait_for_all_requests();

        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
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
            // to start the battle
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
void RestaurantFarmer::run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

    WallClock start = current_time();

    while (true){
        context.wait_for_all_requests();

        SelectionArrowWatcher arrow(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.654308, 0.481553, 0.295529, 0.312621}
        );
        ItemReceiveWatcher item_receive(COLOR_RED, &env.console.overlay(), 1000ms);
        FlatWhiteDialogWatcher dialog0(COLOR_RED, &env.console.overlay());
        BlueDialogWatcher dialog1(COLOR_RED, &env.console.overlay());


        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                while (current_time() - start < 30min){
                    attempt_attack(env, context);
                }
            },
            {
                arrow,
                item_receive,
                dialog0,
                dialog1,
            }
        );

        switch (ret){
        case 0:
            env.log("Detected selection arrow. (unexpected)", COLOR_RED);
            dump_image(env.console.logger(), env.program_info(), env.console.video(), "UnexpectedSelectionArrow");
            stats.errors++;
//            stats.battles++;
            env.update_stats();
//            return;
           continue;

        case 2:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 1:
        case 3:
            env.log("Detected blue dialog. End of battle!");
            stats.battles++;
            env.update_stats();
            return;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Battle took longer than 30 minutes.",
                env.console
            );
        }

    }
}


bool RestaurantFarmer::attempt_attack(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (!MOVE_AI){
        ssf_press_button(context, BUTTON_ZL, 160ms, 800ms, 200ms);
        if (USE_PLUS_MOVES){
            ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
//          pbf_wait(context, 104ms);
        }
        pbf_press_button(context, BUTTON_X, 80ms, 24ms);
        pbf_press_button(context, BUTTON_Y, 80ms, 24ms);
        pbf_press_button(context, BUTTON_B, 80ms, 24ms);
        return true;
    }

    AsyncCommandSession<ProController> command(
        context,
        env.logger(),
        env.realtime_dispatcher(),
        context
    );

    MoveEffectivenessSymbolWatcher move_watcher(COLOR_RED, &env.console.overlay(), 100ms);
    command.dispatch([](ProControllerContext& context){
        pbf_press_button(context, BUTTON_ZL, 10000ms, 0ms);
    });

    int ret = wait_until(
        env.console, context, 1000ms,
        {move_watcher}
    );
    if (ret < 0){
        command.stop_session_and_rethrow();
        context.wait_for(250ms);
        pbf_press_button(context, BUTTON_B, 160ms, 80ms);
        return false;
    }

    MoveEffectivenessSymbol best_type = move_watcher[0];
    Button best_move = BUTTON_X;
    const char* best_string = "Picking Move: Top";
    if (best_type < move_watcher[1]){
        best_type = move_watcher[1];
        best_move = BUTTON_Y;
        best_string = "Picking Move: Left";
    }
    if (best_type < move_watcher[2]){
        best_type = move_watcher[2];
        best_move = BUTTON_A;
        best_string = "Picking Move: Right";
    }
    if (best_type < move_watcher[3]){
//        best_type = move_watcher[3];
        best_move = BUTTON_B;
        best_string = "Picking Move: Bottom";
    }

    env.log(best_string, COLOR_BLUE);

    command.dispatch([&](ProControllerContext& context){
        ssf_press_button(context, BUTTON_ZL, 0ms, 800ms, 200ms);
        if (USE_PLUS_MOVES){
            ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
//            pbf_wait(context, 104ms);
        }
        pbf_press_button(context, best_move, 160ms, 320ms);
    });

    command.wait();

    command.stop_session_and_rethrow();
    return true;
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
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();
    m_stop_after_current.store(false, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_ready();
    ResetOnExit reset_button_on_exit(STOP_AFTER_CURRENT);
    pbf_mash_button(context, BUTTON_B, 1000ms);

//    auto lobby = env.console.video().snapshot();

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_ROUNDS != 0 && stats.battles >= NUM_ROUNDS) {
            m_stop_after_current.store(true, std::memory_order_relaxed);
            STOP_AFTER_CURRENT.set_pressed();
        }
        if (run_lobby(env, context)){
            break;
        }
        run_battle(env, context);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
