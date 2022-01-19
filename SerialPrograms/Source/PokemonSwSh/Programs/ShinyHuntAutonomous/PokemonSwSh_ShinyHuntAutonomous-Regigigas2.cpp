/*  Shiny Hunt Autonomous - Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousRegigigas2_Descriptor::ShinyHuntAutonomousRegigigas2_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousRegigigas2",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Regigigas2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Regigigas2.md",
        "Automatically hunt for shiny Regigigas using video feedback.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousRegigigas2::ShinyHuntAutonomousRegigigas2(const ShinyHuntAutonomousRegigigas2_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        24
    )
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CATCH_TO_OVERWORLD_DELAY(
        "<b>Catch to Overworld Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(REVERSAL_PP);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CATCH_TO_OVERWORLD_DELAY);
}




std::unique_ptr<StatsTracker> ShinyHuntAutonomousRegigigas2::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}



bool ShinyHuntAutonomousRegigigas2::kill_and_return(SingleSwitchProgramEnvironment& env) const{
    pbf_mash_button(env.console, BUTTON_A, 4 * TICKS_PER_SECOND);

    RaidCatchDetector detector(env.console);
    int result = wait_until(
        env, env.console,
        std::chrono::seconds(30),
        { &detector }
    );
    switch (result){
    case 0:
        pbf_press_dpad(env.console, DPAD_DOWN, 10, 0);
        pbf_press_button(env.console, BUTTON_A, 10, CATCH_TO_OVERWORLD_DELAY);
        return true;
    default:
        env.log("Raid Catch Menu not found.", COLOR_RED);
        return false;
    }
}
void ShinyHuntAutonomousRegigigas2::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console,
        Language::None,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        for (uint8_t pp = REVERSAL_PP; pp > 0; pp--){
            env.log("Starting Regigigas Encounter: " + tostr_u_commas(stats.encounters() + 1));

            pbf_mash_button(env.console, BUTTON_A, 18 * TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            {
                StartBattleWatcher detector;
                int result = wait_until(
                    env, env.console,
                    std::chrono::seconds(30),
                    { &detector }
                );
                if (result < 0){
                    stats.add_error();
                    env.update_stats();
                    break;
                }
                env.log("Detected battle start.");
            }

            ShinyDetectionResult result = detect_shiny_battle(
            env, env.console, env.console, env.console,
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

            kill_and_return(env);
        }

        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
        TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(env.console);
        reset_game_from_home_with_inference(
            env, env.console,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }


StopProgram:
    env.update_stats();
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


