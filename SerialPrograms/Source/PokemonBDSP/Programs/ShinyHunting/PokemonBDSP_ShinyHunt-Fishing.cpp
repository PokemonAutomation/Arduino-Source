/*  Shiny Hunt - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_ShinyHunt-Fishing.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyHuntFishing_Descriptor::ShinyHuntFishing_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:ShinyHuntFishing",
        STRING_POKEMON + " BDSP", "Shiny Hunt - Fishing",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ShinyHunt-Fishing.md",
        "Shiny hunt fishing " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ShinyHuntFishing_Descriptor::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
        , m_nothing(m_stats["Nothing"])
        , m_misses(m_stats["Misses"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Nothing"));
        m_display_order.insert(m_display_order.begin() + 2, Stat("Misses"));
    }
    std::atomic<uint64_t>& m_nothing;
    std::atomic<uint64_t>& m_misses;
};
std::unique_ptr<StatsTracker> ShinyHuntFishing_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHuntFishing::ShinyHuntFishing()
    : GO_HOME_WHEN_DONE(false)
    , SHORTCUT("<b>Fishing Shortcut:</b>")
    , ENCOUNTER_BOT_OPTIONS(true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
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

    PA_ADD_OPTION(SHORTCUT);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
}






void ShinyHuntFishing::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHuntFishing_Descriptor::Stats& stats = env.current_stats<ShinyHuntFishing_Descriptor::Stats>();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );
    LeadingShinyTracker lead_tracker(env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    //  Encounter Loop
    while (true){
        env.update_stats();
        pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
        context.wait_for_all_requests();

        {
            ShortDialogWatcher dialog_detector;
            MarkDetector mark_detector(env.console, {0.4, 0.2, 0.2, 0.5});
            StartBattleDetector battle(env.console);
            BattleMenuWatcher battle_menu(BattleType::STANDARD);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [this](ProControllerContext& context){
                    SHORTCUT.run(context, 30 * TICKS_PER_SECOND);
                },
                {
                    {dialog_detector},
                    {mark_detector},
                    {battle_menu},
                }
            );
            switch (ret){
            case 0:
                env.log("Nothing found...", COLOR_ORANGE);
                stats.m_nothing++;
                continue;
            case 1:
                env.log("Hooked something!", COLOR_BLUE);
                pbf_press_button(context, BUTTON_ZL, 10, TICKS_PER_SECOND);
                break;
            case 2:
                env.log("Unexpected battle menu.", COLOR_RED);
                stats.add_error();
                handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT0);
                continue;
            default:
                env.log("Timed out.", COLOR_RED);
                stats.add_error();
                continue;
            }

            //  Wait for dialog after hooking to appear.
            ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(5000),
                {
                    {dialog_detector},
                    {battle_menu},
                }
            );
            switch (ret){
            case 0:
                pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
                break;
            case 1:
                env.log("Unexpected battle menu.", COLOR_RED);
                stats.add_error();
                handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT0);
                continue;
            default:
                env.log("Timed out.", COLOR_RED);
                stats.add_error();
                continue;
            }

            //  Wait for battle to start.
            ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(10000),
                {
                    {battle},
                    {battle_menu},
                }
            );
            switch (ret){
            case 0:
                env.console.log("Battle started!");
                break;
            case 1:
                env.log("Unexpected battle menu.", COLOR_RED);
                stats.add_error();
                handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT0);
                continue;
            default:
                env.log("Missed the hook.", COLOR_ORANGE);
                stats.m_misses++;
                continue;
            }
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

        bool stop = handler.handle_standard_encounter_end_battle(
            result_wild, EXIT_BATTLE_TIMEOUT0
        );
        if (stop){
            break;
        }
        lead_tracker.report_result(result_own.shiny_type);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
