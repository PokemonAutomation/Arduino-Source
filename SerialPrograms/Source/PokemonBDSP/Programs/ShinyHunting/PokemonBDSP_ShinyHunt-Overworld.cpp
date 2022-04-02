/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_ShinyHunt-Overworld.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyHuntOverworld_Descriptor::ShinyHuntOverworld_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:ShinyHuntOverworld",
        STRING_POKEMON + " BDSP", "Shiny Hunt - Overworld",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ShinyHunt-Overworld.md",
        "Shiny hunt overworld " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntOverworld::ShinyHuntOverworld(const ShinyHuntOverworld_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
//        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
//        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(TRIGGER_METHOD);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
//    PA_ADD_OPTION(WATCHDOG_TIMER);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}




struct ShinyHuntOverworld::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
//        , m_resets(m_stats["Resets"])
    {
//        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
//        m_aliases["Unexpected Battles"] = "Errors";
    }
//    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> ShinyHuntOverworld::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void ShinyHuntOverworld::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    Stats& stats = env.stats<Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );
    LeadingShinyTracker lead_tracker(env.console);

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_B, 5, 5);

    //  Encounter Loop
    while (true){
        //  Find encounter.
        bool battle = TRIGGER_METHOD.find_encounter(env);
        if (!battle){
            stats.add_error();
            handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT);
            continue;
        }

        //  Detect shiny.
        DoublesShinyDetection result_wild;
        ShinyDetectionResult result_own;
        detect_shiny_battle(
            env, env.console,
            result_wild, result_own,
            WILD_POKEMON,
            std::chrono::seconds(30)
        );

//        result_wild.shiny_type = ShinyType::UNKNOWN_SHINY;
//        result_wild.left_is_shiny = false;
//        result_wild.right_is_shiny = true;

        bool stop = handler.handle_standard_encounter_end_battle(result_wild, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
        lead_tracker.report_result(result_own.shiny_type);
    }

    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}
















}
}
}
