/*  Jacinthe Infinite Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
//#include "PokemonLZA/Inference/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA/Programs/PokemonLZA_TrainerBattle.h"
#include "PokemonLZA_JacintheInfiniteFarmer.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


// Jacinthe infinite battle flow:
// - Button A overworld
// - Flat white dialog x 2
// - Selection arrow for three menuitem: "give a try", "can you give me spiel again", "I'll pass"
// - Flat white excited dialog
// - Flat white dialog
// - Black screen
// - Enter pre-battle animation
// - Pre-battle dialog (no background, white arrow)
// - Battle
//   - Aurorus uses protect first turn
//   - Mega evolve Clefable
//   - Clefable uses protect first turn
// - Given +8100 poke dollar
// - Flat white dialog
// - Black screen
// - Flat white dialog
// - Selection arrow for two item: "let's start", "no thanks"
// - If choose let's start:
//   - Flat white dialog
//   - Pre-battle dialog
//   - <loop> ...
// - If choose no thanks:
//  - Flat white dialg
//  - Selection arrow for two item: "let's keep going", "I'm done"
//  - choose 2nd
//  - Flat white dialog x3
//  - Button A overworld <finish battle>


JacintheInfiniteFarmer_Descriptor::JacintheInfiniteFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:JacintheInfiniteFarmer",
        STRING_POKEMON + " LZA", "Jacinthe Infinite Farmer",
        "Programs/PokemonLZA/JacintheInfiniteFarmer.html",
        "Farm Jacinthe Infinite for exp and money.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class JacintheInfiniteFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : rounds(m_stats["Rounds"])
        , defeats(m_stats["Defeats"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Defeats", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Battles"] = "Rounds";
    }

    std::atomic<uint64_t>& rounds;
    std::atomic<uint64_t>& defeats;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> JacintheInfiniteFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


JacintheInfiniteFarmer::~JacintheInfiniteFarmer(){
    STOP_AFTER_CURRENT.remove_listener(*this);
}

JacintheInfiniteFarmer::JacintheInfiniteFarmer()
    : m_stop_after_current(false)
    , NUM_ROUNDS(
        "<b>Number of Rounds to Run:</b><br>"
        "Zero will run until 'Stop after Current Round' is pressed or the program is manually stopped.</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100,
        0
    )
    , GO_HOME_WHEN_DONE(false)
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
    PA_ADD_OPTION(NOTIFICATIONS);

    STOP_AFTER_CURRENT.set_idle();
    STOP_AFTER_CURRENT.add_listener(*this);
}



JacintheInfiniteFarmer::StopButton::StopButton()
    : ButtonOption(
      "<b>Stop after current round:",
      "Stop after current round",
      0, 16
    )
{}
void JacintheInfiniteFarmer::StopButton::set_idle(){
    this->set_enabled(false);
    this->set_text("Stop after Current Round");
}
void JacintheInfiniteFarmer::StopButton::set_ready(){
    this->set_enabled(true);
    this->set_text("Stop after Current Round");
}
void JacintheInfiniteFarmer::StopButton::set_pressed(){
    this->set_enabled(false);
    this->set_text("Program will stop after current round...");
}



bool JacintheInfiniteFarmer::talk_to_jacinthe(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    JacintheInfiniteFarmer_Descriptor::Stats& stats = env.current_stats<JacintheInfiniteFarmer_Descriptor::Stats>();

    bool seen_selection_arrow = false;
    bool confirm_entering_battle = false;
    bool seen_flat_white_dialog = false;

    TransparentBattleDialogWatcher transparent_battle_dialog(COLOR_WHITE, &env.console.overlay());
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
            {0.543, 0.508, 0.365, 0.253}
        );
        FlatWhiteDialogWatcher dialog(COLOR_RED, &env.console.overlay());
        BlackScreenWatcher black_screen(COLOR_BLACK);

        std::vector<PeriodicInferenceCallback> callbacks = {
            buttonA,
            arrow,
            dialog
        };
        if (seen_selection_arrow && confirm_entering_battle){
            // seen selection arrow means we may now enter battle, which transitions with a black screen
            callbacks.push_back(black_screen);
        }

        int ret = wait_until(
            env.console, context,
            10000ms,
            callbacks
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            // There cases where the function detects A button:
            // - At start of the program, press A to start talking to Jacinthe
            // - When stopping the battle, the program fininshes all the dialog with Jacinethe and returns
            //   to the overworld, where button A appears.
            // - When we lose to Jacinthe, we have some dialog after the battle and returns to the overworld.
            env.log("Detected A button.");
            env.console.overlay().add_log("Button A Detected");
            if (m_stop_after_current.load(std::memory_order_relaxed)){
                return true; // true means the program should stop
            }
            
            if (seen_flat_white_dialog){
                // we shouldn't stop the program but we find A button. This means we lost.
                stats.defeats++;
                env.update_stats();
            }
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 1:
            env.log("Detected selection arrow.");
            seen_selection_arrow = true;
            // This is when Jacinthe is asking whether you want
            // to start the battle or continue the battle
            if (m_stop_after_current.load(std::memory_order_relaxed)){
                env.console.overlay().add_log("Dialog Choice: Cancel");
                pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            } else{
                confirm_entering_battle = true;
                // confirm entering battle
                env.console.overlay().add_log("Dialog Choice: Confirm");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            }
            continue;
        case 2:
            env.log("Detected white dialog.");
            // env.console.overlay().add_log("Advance Dialog");
            seen_flat_white_dialog = true;
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;
        
        case 3:
            env.log("Detected black screen.");
            env.console.overlay().add_log("Transition to Battle");

            ret = wait_until(
                env.console, context, 20s, {transparent_battle_dialog}
            );
            if (ret != 0){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "talk_to_jacinthe(): Does not detect transparent battle dialog 20 sec after black screen.",
                    env.console
                );
            }
            // mash B to clear up pre-battle transparency dialog
            env.log("Clearing transparent battle dialog");
            env.console.overlay().add_log("Clear Battle Dialog");
            pbf_mash_button(context, BUTTON_B, 6s);
            context.wait_for_all_requests();
            // battle starts
            return false;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "talk_to_jacinthe(): No recognized state after 60 seconds.",
                env.console
            );
        }
    }
}
void JacintheInfiniteFarmer::run_round(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    JacintheInfiniteFarmer_Descriptor::Stats& stats = env.current_stats<JacintheInfiniteFarmer_Descriptor::Stats>();

    WallClock start = current_time();

    while (true){
        context.wait_for_all_requests();

        FlatWhiteDialogWatcher dialog(COLOR_RED, &env.console.overlay(), 250ms);

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                while (current_time() - start < 30min){
                    const bool allow_button_B_press = false;
                    attempt_one_attack(env, context, MOVE_AI, USE_PLUS_MOVES, allow_button_B_press);
                }
            },
            {
                dialog,
            }
        );

        switch (ret){
        case 0:
            env.log("Detected white dialog.");
            env.console.overlay().add_log("Post-Battle Dialog");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            stats.rounds++;
            env.update_stats();
            return;

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


class JacintheInfiniteFarmer::ResetOnExit{
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

void JacintheInfiniteFarmer::on_press(){
    global_logger_tagged().log("Stop after current requested...");
    m_stop_after_current.store(true, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_pressed();
}

void JacintheInfiniteFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    JacintheInfiniteFarmer_Descriptor::Stats& stats = env.current_stats<JacintheInfiniteFarmer_Descriptor::Stats>();
    m_stop_after_current.store(false, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_ready();
    ResetOnExit reset_button_on_exit(STOP_AFTER_CURRENT);
    pbf_mash_button(context, BUTTON_B, 1000ms);

//    auto lobby = env.console.video().snapshot();

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_ROUNDS != 0 && stats.rounds >= NUM_ROUNDS) {
            m_stop_after_current.store(true, std::memory_order_relaxed);
            STOP_AFTER_CURRENT.set_pressed();
        }
        if (talk_to_jacinthe(env, context)){
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
