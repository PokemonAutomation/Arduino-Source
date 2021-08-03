/*  Shiny Hunt Autonomous - Berry Tree
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
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousBerryTree_Descriptor::ShinyHuntAutonomousBerryTree_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousBerryTree",
        "Shiny Hunt Autonomous - Berry Tree",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-BerryTree",
        "Automatically hunt for shiny berry tree " + STRING_POKEMON + " using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousBerryTree::ShinyHuntAutonomousBerryTree(const ShinyHuntAutonomousBerryTree_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(m_name_reader)
    , FILTER(false, true)
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
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

    m_options.emplace_back(&NOTIFICATION_LEVEL, "NOTIFICATION_LEVEL");

    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_TIMEOUT, "EXIT_BATTLE_TIMEOUT");
    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
    }
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousBerryTree::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(true));
}




void ShinyHuntAutonomousBerryTree::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

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

    uint8_t year = MAX_YEAR;
    while (true){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
        home_roll_date_enter_game_autorollback(env.console, &year);
        pbf_mash_button(env.console, BUTTON_B, 90);
        env.console.botbase().wait_for_all_requests();

        {
            InterruptableCommandSession commands(env.console);

            StandardBattleMenuDetector battle_menu_detector(env.console);
            battle_menu_detector.register_command_stop(commands);

            StartBattleDetector start_battle_detector(env.console);
            start_battle_detector.register_command_stop(commands);

            AsyncVisualInferenceSession inference(env, env.console);
            inference += battle_menu_detector;
            inference += start_battle_detector;

            commands.run([](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_A, 60 * TICKS_PER_SECOND);
                context.botbase().wait_for_all_requests();
            });

            if (battle_menu_detector.triggered()){
                env.log("Unexpected battle menu.", Qt::red);
                stats.add_error();
                env.update_stats();
                pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
                run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
                continue;
            }
            if (start_battle_detector.triggered()){
                env.log("Battle started!");
            }else{
                stats.add_error();
                env.update_stats();
                env.log("Timed out.");
                continue;
            }
        }

        //  Detect shiny.
        ShinyType shininess = detect_shiny_battle(
            env, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_runaway(shininess, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    home_to_date_time(env.console, false, false);
    pbf_press_button(env.console, BUTTON_A, 5, 5);
    pbf_press_button(env.console, BUTTON_A, 5, 10);
    pbf_press_button(env.console, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY);

    if (!GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}




}
}
}
