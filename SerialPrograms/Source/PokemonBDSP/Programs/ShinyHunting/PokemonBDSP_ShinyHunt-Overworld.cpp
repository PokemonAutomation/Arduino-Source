/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP_ShinyHunt-Overworld.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyHuntOverworld_Descriptor::ShinyHuntOverworld_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:ShinyHuntOverworld",
        STRING_POKEMON + " BDSP", "Shiny Hunt - Overworld",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ShinyHunt-Overworld.md",
        "Shiny hunt overworld " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ShinyHuntOverworld_Descriptor::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
//        , m_resets(m_stats["Resets"])
    {
//        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
//        m_aliases["Unexpected Battles"] = "Errors";
    }
//    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> ShinyHuntOverworld_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ShinyHuntOverworld::ShinyHuntOverworld()
    : GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true)
    , RESET_GAME_WHEN_ERROR(
        "<b>Reset Game in Case of Error:</b><br>"
        "When the program encounters an error, whether to reset the game to fix it.<br>"
        "<b>Make sure you have set \"Throw balls. Save if caught.\" in Option Overrides if you don't stop when finding a shiny.</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
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

    PA_ADD_OPTION(TRIGGER_METHOD);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(RESET_GAME_WHEN_ERROR);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
//    PA_ADD_OPTION(WATCHDOG_TIMER0);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
}






void ShinyHuntOverworld::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    OverlayLogTextScope overlay_log_text_scope(env.console.overlay());
    ShinyHuntOverworld_Descriptor::Stats& stats = env.current_stats<ShinyHuntOverworld_Descriptor::Stats>();
    env.update_stats();

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
        //  Find encounter.
        try{
            bool battle = TRIGGER_METHOD.find_encounter(env.console, context);
            if (!battle){
                stats.add_error();
                handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT0);
                continue;
            }
            env.console.overlay().add_log("Battle started", COLOR_GREEN);

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

    //        result_wild.shiny_type = ShinyType::UNKNOWN_SHINY;
    //        result_wild.left_is_shiny = false;
    //        result_wild.right_is_shiny = true;

            bool stop = handler.handle_standard_encounter_end_battle(
                result_wild, EXIT_BATTLE_TIMEOUT0
            );

            if (stop){
                break;
            }
            lead_tracker.report_result(result_own.shiny_type);

        }catch (OperationFailedException& e){
            if (!RESET_GAME_WHEN_ERROR){
                throw;
            }
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            stats.add_error();
            go_home(env.console, context);
            if (!reset_game_from_home(
                env, env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            )){
                stats.add_error();
                continue;
            }
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}
















}
}
}
