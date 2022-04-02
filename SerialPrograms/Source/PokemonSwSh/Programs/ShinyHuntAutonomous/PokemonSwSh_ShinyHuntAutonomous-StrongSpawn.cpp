/*  Shiny Hunt Autonomous - Strong Spawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousStrongSpawn_Descriptor::ShinyHuntAutonomousStrongSpawn_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousStrongSpawn",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Strong Spawn",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-StrongSpawn.md",
        "Automatically hunt for shiny strong spawns using video feedback.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousStrongSpawn::ShinyHuntAutonomousStrongSpawn(const ShinyHuntAutonomousStrongSpawn_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
//    , m_advanced_options(
//        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
//    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

//    if (PERSISTENT_SETTINGS().developer_mode){
//        PA_ADD_STATIC(m_advanced_options);
//    }
}




std::unique_ptr<StatsTracker> ShinyHuntAutonomousStrongSpawn::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}



void ShinyHuntAutonomousStrongSpawn::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(context);

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, context, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        uint32_t now = system_clock(context);
        if (TIME_ROLLBACK_HOURS > 0 && now - last_touch >= PERIOD){
            rollback_hours_from_home(context, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            last_touch += PERIOD;
        }
        reset_game_from_home_with_inference(
            env, context, env.console,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        context.wait_for_all_requests();

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env, context, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter(result);
        if (stop){
            break;
        }

        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}

