/*  Shiny Hunt - Shaymin Runaway
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_ShinyHunt-Shaymin.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyHuntShaymin_Descriptor::ShinyHuntShaymin_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:ShinyHuntShaymin",
        STRING_POKEMON + " BDSP", "Shiny Hunt - Shaymin",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ShinyHunt-Shaymin.md",
        "Shiny hunt Shaymin using the runaway method.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntShaymin_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new PokemonSwSh::ShinyHuntTracker(false));
}


ShinyHuntShaymin::ShinyHuntShaymin()
    : GO_HOME_WHEN_DONE(false)
//    , SHORTCUT("<b>Bike Shortcut:</b>")
    , ENCOUNTER_BOT_OPTIONS(false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
//        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
//        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "10 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
//    PA_ADD_OPTION(SHORTCUT);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
//    PA_ADD_OPTION(WATCHDOG_TIMER);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



bool ShinyHuntShaymin::start_encounter(SingleSwitchProgramEnvironment& env, BotBaseContext& context) const{
    context.wait_for_all_requests();
    {
        BattleMenuWatcher battle_menu_detector(BattleType::STANDARD);
        ShortDialogWatcher dialog_detector;
        int result = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                while (true){
                    for (size_t c = 0; c < 5; c++){
                        pbf_press_button(context, BUTTON_ZL, 20, 105);
                    }
//                    pbf_mash_button(context, BUTTON_ZL, 5 * TICKS_PER_SECOND);
                }
            },
            {
                {battle_menu_detector},
                {dialog_detector},
            }
        );
        switch (result){
        case 0:
            env.console.log("Unexpected Battle.", COLOR_RED);
            return false;
        case 1:
            env.console.log("Talked to Shaymin!");
            break;
        }
    }
    {
        BattleMenuWatcher battle_menu_detector(BattleType::STANDARD);
        StartBattleDetector start_battle_detector(env.console);
        int result = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                while (true){
                    for (size_t c = 0; c < 5; c++){
                        pbf_press_button(context, BUTTON_ZL, 20, 105);
                    }
//                    pbf_mash_button(context, BUTTON_ZL, 5 * TICKS_PER_SECOND);
                }
            },
            {
                {battle_menu_detector},
                {start_battle_detector},
            }
        );
        switch (result){
        case 0:
            env.console.log("Unexpected Battle.", COLOR_RED);
            return false;
        case 1:
            env.console.log("Battle started!");
            break;
        }
    }
    return true;
}

void ShinyHuntShaymin::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    PokemonSwSh::ShinyHuntTracker& stats = env.current_stats<PokemonSwSh::ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        Language::None,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );
    LeadingShinyTracker lead_tracker(env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    //  Encounter Loop
    while (true){
        //  Find encounter.
        bool battle = start_encounter(env, context);
        if (!battle){
            stats.add_error();
            handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT);
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

        bool stop = handler.handle_standard_encounter_end_battle(result_wild, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
        lead_tracker.report_result(result_own.shiny_type);

        //  Clear dialogs.
        pbf_mash_button(context, BUTTON_B, 75);

        //  Hop on bike, ride down to seabreak path
//        SHORTCUT.run(env.console, 0);
        pbf_move_left_joystick(context, 128, 255, 360, 0);
        pbf_move_left_joystick(context, 128, 0, 370, 0);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

}
}
}
