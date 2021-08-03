/*  Shiny Hunt Autonomous - Strong Spawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
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
        "Shiny Hunt Autonomous - Strong Spawn",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-StrongSpawn",
        "Automatically hunt for shiny strong spawns using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousStrongSpawn::ShinyHuntAutonomousStrongSpawn(const ShinyHuntAutonomousStrongSpawn_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(m_name_reader)
    , FILTER(true, false)
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");

    m_options.emplace_back(&LANGUAGE, "LANGUAGE");
    m_options.emplace_back(&FILTER, "FILTER");

    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
    m_options.emplace_back(&NOTIFICATION_LEVEL, "NOTIFICATION_LEVEL");

    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(&m_advanced_options, "");
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
    }
}




std::unique_ptr<StatsTracker> ShinyHuntAutonomousStrongSpawn::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}



void ShinyHuntAutonomousStrongSpawn::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        &m_name_reader, LANGUAGE,
        stats,
        FILTER,
        VIDEO_ON_SHINY,
        NOTIFICATION_LEVEL
    );

    while (true){
        uint32_t now = system_clock(env.console);
        if (TIME_ROLLBACK_HOURS > 0 && now - last_touch >= PERIOD){
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            last_touch += PERIOD;
        }
        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        env.console.botbase().wait_for_all_requests();

        //  Detect shiny.
        ShinyType shininess = detect_shiny_battle(
            env, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter(shininess);
        if (stop){
            break;
        }

        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

