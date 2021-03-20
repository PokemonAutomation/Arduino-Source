/*  ShinyHuntUnattended-Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHunt-Regi.h"
#include "PokemonSwSh_ShinyHuntUnattended-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntUnattendedRegi::ShinyHuntUnattendedRegi()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Unattended - Regi",
        "NativePrograms/ShinyHuntUnattended-Regi.md",
        "Hunt for shiny Regis. Stop when a shiny is found."
    )
    , START_TO_RUN_DELAY(
        "<b>Start to Run Delay:</b><br>This needs to be carefully calibrated.",
        "1990"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CORRECTION_INTERVAL(
        "<b>Correction Interval:</b><br>Periodically leave the building to fix broken lights. Zero disables these corrections.",
        20
    )
    , TRANSITION_DELAY(
        "<b>Transition Delay:</b><br>Time to enter/exit the building.",
        "5 * TICKS_PER_SECOND"
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&START_TO_RUN_DELAY, "START_TO_RUN_DELAY");
    m_options.emplace_back(&REGI_NAME, "REGI_NAME");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&CORRECTION_INTERVAL, "CORRECTION_INTERVAL");
    m_options.emplace_back(&TRANSITION_DELAY, "TRANSITION_DELAY");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
}



void ShinyHuntUnattendedRegi::program(SingleSwitchProgramEnvironment& env) const{
//    BotBase& botbase = env.console;

//    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);

    uint32_t last_touch = system_clock() - TOUCH_DATE_INTERVAL;
    uint16_t correct_count = 0;
    for (uint32_t c = 0; ; c++){
        //  Auto-correction.
        bool correct = CORRECTION_INTERVAL > 0 && correct_count >= CORRECTION_INTERVAL;
        move_to_corner(env, correct, TRANSITION_DELAY);
        if (correct){
            correct_count = 0;
        }

        //  Touch the date.
        if (TOUCH_DATE_INTERVAL > 0 && system_clock() - last_touch >= TOUCH_DATE_INTERVAL){
            env.logger.log("Touching date to prevent rollover.");
            pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            touch_date_from_home(SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += TOUCH_DATE_INTERVAL;
        }


        //  Do the light puzzle.
        run_regi_light_puzzle(env, REGI_NAME, c);

        pbf_press_button(BUTTON_A, 10, 100);
        pbf_press_button(BUTTON_A, 10, 100);
        if (START_TO_RUN_DELAY >= 500){
            //  Extra A press to fix A parity if the lights were messed up.
            pbf_press_button(BUTTON_A, 10, 500);
            pbf_press_button(BUTTON_A, 10, START_TO_RUN_DELAY - 500);
        }else{
            pbf_press_button(BUTTON_A, 10, START_TO_RUN_DELAY);
        }

        //  Run away if not shiny.
        run_away_with_lights();

        //  Enter Pokemon menu if shiny.
        enter_summary(true);

        correct_count++;
    }

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
}


}
}
}



