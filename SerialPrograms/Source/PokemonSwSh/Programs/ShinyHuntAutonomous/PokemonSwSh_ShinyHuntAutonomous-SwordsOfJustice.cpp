/*  Shiny Hunt Autonomous - Swords Of Justice
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
#include "CommonFramework/Inference/VisualInferenceWait.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousSwordsOfJustice_Descriptor::ShinyHuntAutonomousSwordsOfJustice_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousSwordsOfJustice",
        "Shiny Hunt Autonomous - Swords Of Justice",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-SwordsOfJustice",
        "Automatically hunt for shiny Sword of Justice using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousSwordsOfJustice::ShinyHuntAutonomousSwordsOfJustice(const ShinyHuntAutonomousSwordsOfJustice_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(m_name_reader)
    , AIRPLANE_MODE(
        "<b>Airplane Mode:</b><br>Enable if airplane mode is on.",
        false
    )
    , FILTER(true, false)
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
    , POST_BATTLE_MASH_TIME(
        "<b>Post-Battle Mash:</b><br>After each battle, mash B for this long before entering the camp.",
        "1 * TICKS_PER_SECOND"
    )
    , ENTER_CAMP_DELAY(
        "<b>Enter Camp Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");

    m_options.emplace_back(&LANGUAGE, "LANGUAGE");
    m_options.emplace_back(&AIRPLANE_MODE, "AIRPLANE_MODE");
    m_options.emplace_back(&FILTER, "FILTER");

    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
    m_options.emplace_back(&NOTIFICATION_LEVEL, "NOTIFICATION_LEVEL");

    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_TIMEOUT, "EXIT_BATTLE_TIMEOUT");
    m_options.emplace_back(&POST_BATTLE_MASH_TIME, "POST_BATTLE_MASH_TIME");
    m_options.emplace_back(&ENTER_CAMP_DELAY, "ENTER_CAMP_DELAY");
    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
    }
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousSwordsOfJustice::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}




void ShinyHuntAutonomousSwordsOfJustice::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
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
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        //  Trigger encounter.
        pbf_mash_button(env.console, BUTTON_B, POST_BATTLE_MASH_TIME);
        pbf_press_button(env.console, BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
        pbf_press_button(env.console, BUTTON_A, 10, ENTER_CAMP_DELAY);
        if (AIRPLANE_MODE){
            pbf_press_button(env.console, BUTTON_A, 10, 100);
            pbf_press_button(env.console, BUTTON_A, 10, 100);
        }
        pbf_press_button(env.console, BUTTON_X, 10, 50);
        pbf_press_dpad(env.console, DPAD_LEFT, 10, 10);
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        pbf_press_button(env.console, BUTTON_A, 10, 0);
        env.console.botbase().wait_for_all_requests();

        {
            //  Wait for start of battle.
            StandardBattleMenuDetector battle_menu_detector(env.console);
            StartBattleDetector start_back_detector(env.console);
            VisualInferenceWait inference(env, env.console, std::chrono::seconds(30));
            inference += battle_menu_detector;
            inference += start_back_detector;
            inference.run();
        }

        //  Detect shiny.
        ShinyType shininess = detect_shiny_battle(
            env, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );
//        shininess = ShinyDetection::SQUARE_SHINY;

        bool stop = handler.handle_standard_encounter_runaway(shininess, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
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
