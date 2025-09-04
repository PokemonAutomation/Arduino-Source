/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_LegendaryReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


LegendaryReset_Descriptor::LegendaryReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:LegendaryReset",
        STRING_POKEMON + " BDSP", "Legendary Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/LegendaryReset.md",
        "Shiny hunt a standing legendary " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> LegendaryReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new PokemonSwSh::ShinyHuntTracker(false));
}


LegendaryReset::LegendaryReset()
    : GO_HOME_WHEN_DONE(false)
    , WALK_UP(
        "<b>Walk Up:</b><br>Walk up while mashing A to trigger encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , ENCOUNTER_BOT_OPTIONS(false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(WALK_UP);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void LegendaryReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PokemonSwSh::ShinyHuntTracker& stats = env.current_stats<PokemonSwSh::ShinyHuntTracker>();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );
    LeadingShinyTracker lead_tracker(env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    bool reset = false;
    while (true){
        env.update_stats();

        if (reset){
            go_home(env.console, context);
            if (!reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST)){
                stats.add_error();
                continue;
            }
        }
        reset = true;

        StartBattleDetector start_battle(env.console);
        BattleMenuWatcher battle_menu(BattleType::STANDARD);

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [this](ProControllerContext& context){
                size_t stop = WALK_UP ? 30 : 60;
                for (size_t c = 0; c < stop; c++){
                    if (WALK_UP){
                        pbf_move_left_joystick(context, 128, 0, 125, 0);
                    }
                    pbf_mash_button(context, BUTTON_ZL, 125);
                }
            },
            {
                {start_battle},
                {battle_menu},
            }
        );
        switch (ret){
        case 0:
            env.log("Battle started!");
            break;
        case 1:
            env.log("Unexpected battle menu.", COLOR_RED);
            continue;
        default:
            env.log("Timed out.", COLOR_RED);
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
        lead_tracker.report_result(result_own.shiny_type);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
