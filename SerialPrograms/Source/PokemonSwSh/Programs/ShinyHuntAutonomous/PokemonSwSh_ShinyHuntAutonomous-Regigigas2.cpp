/*  Shiny Hunt Autonomous - Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousRegigigas2_Descriptor::ShinyHuntAutonomousRegigigas2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousRegigigas2",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Regigigas2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Regigigas2.md",
        "Automatically hunt for shiny Regigigas using video feedback.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntAutonomousRegigigas2_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}



ShinyHuntAutonomousRegigigas2::ShinyHuntAutonomousRegigigas2()
    : GO_HOME_WHEN_DONE(false)
    , REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        LockMode::LOCK_WHILE_RUNNING,
        24
    )
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(REVERSAL_PP);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool ShinyHuntAutonomousRegigigas2::kill_and_return(VideoStream& stream, ProControllerContext& context) const{
    pbf_mash_button(context, BUTTON_A, 4000ms);

    while (true){
        RaidCatchDetector detector(stream.overlay());
        YCommIconDetector overworld(true);
        context.wait_for_all_requests();
        int result = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {
                detector,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(250));
        switch (result){
        case 0:
            ssf_press_dpad_ptv(context, DPAD_DOWN, 80ms);
            pbf_mash_button(context, BUTTON_A, 250ms);
            break;
        case 1:
            stream.log("Detected overworld.");
            return true;
        default:
            stream.log("No state detected after 30 seconds.", COLOR_RED);
            return false;
        }
    }
}
void ShinyHuntAutonomousRegigigas2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.current_stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        Language::None,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        for (uint8_t pp = REVERSAL_PP; pp > 0; pp--){
            env.log("Starting Regigigas Encounter: " + tostr_u_commas(stats.encounters() + 1));

            pbf_mash_button(context, BUTTON_A, 5s);
            context.wait_for_all_requests();

            {
                StartBattleWatcher detector;
                int result = wait_until(
                    env.console, context,
                    std::chrono::seconds(30),
                    {{detector}}
                );
                if (result < 0){
                    stats.add_error();
                    env.update_stats();
                    if (NOTIFICATION_ERROR_RECOVERABLE.ok_to_send_now(env.logger())){
                        dump_image(
                            env.logger(), env.program_info(), env.console,
                            "Start of battle not detected after 30 seconds."
                        );
                    }
                    break;
                }
                env.log("Detected battle start.");
            }

            ShinyDetectionResult result = detect_shiny_battle(
                env.console, context,
                SHINY_BATTLE_RAID,
                std::chrono::seconds(30)
            );
//            shininess = ShinyDetection::STAR_SHINY;

            bool stop = handler.handle_standard_encounter(result);
            if (stop){
                goto StopProgram;
            }

            if (result.shiny_type == ShinyType::UNKNOWN){
                stats.add_error();
                env.update_stats();
                break;
            }

            kill_and_return(env.console, context);
        }

        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
        TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(env.console, context);
        reset_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }


StopProgram:
    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}


