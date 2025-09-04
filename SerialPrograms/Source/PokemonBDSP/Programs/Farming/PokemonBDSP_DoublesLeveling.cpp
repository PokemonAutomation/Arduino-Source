/*  Double Battle Leveling
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_DoublesLeveling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


DoublesLeveling_Descriptor::DoublesLeveling_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:DoublesLeveling",
        STRING_POKEMON + " BDSP", "Double Battle Leveling",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/DoublesLeveling.md",
        "Level up your party by spamming spread moves in a double battle with a partner that heals you forever.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct DoublesLeveling_Descriptor::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
//        , m_resets(m_stats["Resets"])
    {
//        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
//        m_aliases["Unexpected Battles"] = "Errors";
    }
//    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> DoublesLeveling_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




DoublesLeveling::DoublesLeveling()
    : GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT0(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "10 s"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(ON_LEARN_MOVE);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
//    PA_ADD_OPTION(WATCHDOG_TIMER0);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
}







bool DoublesLeveling::battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DoublesLeveling_Descriptor::Stats& stats = env.current_stats<DoublesLeveling_Descriptor::Stats>();

    env.log("Starting battle!");

    //  State Machine
    for (size_t c = 0; c < 5;){
        context.wait_for_all_requests();

        BattleMenuWatcher battle_menu(BattleType::STANDARD);
        EndBattleWatcher end_battle;
        SelectionArrowFinder learn_move(env.console, {0.50, 0.62, 0.40, 0.18}, COLOR_YELLOW);
        FrozenImageDetector overworld(std::chrono::seconds(5), 10);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {
                battle_menu,
                end_battle,
                learn_move,
                overworld,
            }
        );
        switch (ret){
        case 0:
            env.log("Battle menu detected!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_ZL, 5 * TICKS_PER_SECOND);
            c++;
            break;
        case 1:
            env.log("Battle finished!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 250);
            return false;
        case 2:
            env.log("Detected move learn!", COLOR_BLUE);
            if (ON_LEARN_MOVE == OnLearnMove::DONT_LEARN){
                pbf_move_right_joystick(context, 128, 255, 20, 105);
                pbf_press_button(context, BUTTON_ZL, 20, 105);
                break;
            }
            return true;
        case 3:
            env.log("Detected possible overworld!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 250);
            return false;
        default:
            env.log("Timed out.", COLOR_RED);
            stats.add_error();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Timed out after 2 minutes.",
                env.console
            );
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "No progress detected after 5 battle menus. Are you out of PP?",
        env.console
    );
}



void DoublesLeveling::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DoublesLeveling_Descriptor::Stats& stats = env.current_stats<DoublesLeveling_Descriptor::Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    //  Encounter Loop
    while (true){
        //  Find encounter.
        bool battle = TRIGGER_METHOD.find_encounter(env.console, context);
        if (!battle){
            // Unexpected battle: detect battle menu but not battle starting animation.
            stats.add_error();
            handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT0);
            continue;
        }

        //  Detect shiny.
        DoublesShinyDetection result_wild;
        ShinyDetectionResult result_own;
        detect_shiny_battle(
            env, env.console, context,
            result_wild, result_own,
            NOTIFICATION_ERROR_RECOVERABLE,
            WILD_POKEMON,
            std::chrono::seconds(30),
            ENCOUNTER_BOT_OPTIONS.USE_SOUND_DETECTION
        );

        bool stop = handler.handle_standard_encounter(result_wild);
        if (stop){
            break;
        }

        if (this->battle(env, context)){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}









}
}
}
