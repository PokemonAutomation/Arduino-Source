/*  Shiny Hunt Autonomous - Berry Tree
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh_EncounterTracker.h"
#include "PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousBerryTree::ShinyHuntAutonomousBerryTree()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - Berry Tree",
        "SerialPrograms/ShinyHuntAutonomous-BerryTree.md",
        "Automatically hunt for shiny berry tree " + STRING_POKEMON + " using video feedback."
    )
    , GO_HOME_WHEN_DONE(
        "<b>Go Home when Done:</b><br>After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        true
    )
    , REQUIRE_SQUARE(
        "<b>Require Square:</b><br>Stop only for a square shiny. Run from star shinies.",
        false
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_MASH_TIME(
        "<b>Exit Battle Time:</b><br>After running, wait this long to return to overworld.",
        "6 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");
    m_options.emplace_back(&REQUIRE_SQUARE, "REQUIRE_SQUARE");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
}






std::string ShinyHuntAutonomousBerryTree::Stats::stats() const{
    std::string str;
    str += str_encounters();
    str += " - Timeouts: " + tostr_u_commas(m_timeouts);
    str += str_shinies();
    return str;
}

void ShinyHuntAutonomousBerryTree::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    Stats stats;
    StandardEncounterTracker tracker(stats, env.console, REQUIRE_SQUARE, EXIT_BATTLE_MASH_TIME);

    uint8_t year = MAX_YEAR;
    while (true){
        stats.log_stats(env, env.logger);

        home_roll_date_enter_game_autorollback(&year);
        pbf_mash_button(BUTTON_B, 90);
        env.console.botbase().wait_for_all_requests();

        {
            StartBattleDetector detector(env.console, std::chrono::seconds(60));

            //  Detect start of battle.
            bool timed_out = false;
            do{
                if (detector.has_timed_out()){
                    env.logger.log("ScreenChangeDetector: Timed out.", Qt::red);
                    stats.m_timeouts++;
                    timed_out = true;
                    break;
                }
                pbf_mash_button(BUTTON_A, 10);
                env.console.botbase().wait_for_all_requests();
            }while (!detector.detect(env.console.video().snapshot()));

            pbf_mash_button(BUTTON_B, 5 * TICKS_PER_SECOND);
            if (timed_out){
                pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
                continue;
            }
        }

        //  Detect shiny.
        ShinyDetection detection = detect_shiny_battle(
            env, env.console, env.logger,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        if (tracker.process_result(detection)){
            break;
        }
        if (detection == ShinyDetection::NO_BATTLE_MENU){
            stats.m_timeouts++;
            pbf_mash_button(BUTTON_B, TICKS_PER_SECOND);
            tracker.run_away();
        }

        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
    }

    stats.log_stats(env, env.logger);

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    home_to_date_time(false, false);
    pbf_press_button(BUTTON_A, 5, 5);
    pbf_press_button(BUTTON_A, 5, 10);
    pbf_press_button(BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY);

    if (!GO_HOME_WHEN_DONE){
        pbf_press_button(BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }

    end_program_callback();
    end_program_loop();
}




}
}
}
