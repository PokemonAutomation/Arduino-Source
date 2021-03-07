/*  ShinyHuntUnattended-Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
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
        10
    )
    , TRANSITION_DELAY(
        "<b>Transition Delay:</b><br>Time to enter/exit the building.",
        "3 * TICKS_PER_SECOND"
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




void regirock(void){
    pbf_move_left_joystick(0, 232, 169, 0);
    pbf_move_left_joystick(128, 255, 135, 0);
    pbf_move_left_joystick(0, 128, 109, 0);
    pbf_move_left_joystick(128, 0, 140, 0);
    pbf_move_left_joystick(226, 255, 90, 0);
    pbf_move_left_joystick(128, 0, 20, 0);
    pbf_mash_button(BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(128, 0, 200, 0);
}
void regice(void){
    pbf_move_left_joystick(80, 255, 182, 0);
    pbf_move_left_joystick(0, 128, 114, 0);
    pbf_move_left_joystick(128, 255, 56, 0);
    pbf_move_left_joystick(32, 0, 76, 0);
    pbf_move_left_joystick(0, 128, 54, 0);
    pbf_move_left_joystick(255, 68, 154, 0);
    pbf_move_left_joystick(128, 0, 20, 0);
    pbf_mash_button(BUTTON_A, 5 * TICKS_PER_SECOND - 20);
    pbf_move_left_joystick(128, 0, 170, 0);
}
void registeel(void){
    pbf_move_left_joystick(0, 232, 169, 0);
    pbf_move_left_joystick(192, 255, 64, 0);
    pbf_move_left_joystick(64, 255, 64, 0);
    pbf_move_left_joystick(0, 128, 110, 0);
    pbf_move_left_joystick(64, 0, 68, 0);
    pbf_move_left_joystick(192, 0, 66, 0);
    pbf_move_left_joystick(230, 255, 90, 0);
    pbf_move_left_joystick(128, 0, 50, 0);
    pbf_mash_button(BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(128, 0, 250, 0);
}
void regieleki(void){
    pbf_move_left_joystick(16, 255, 160, 0);
    pbf_move_left_joystick(128, 255, 90, 0);
    pbf_move_left_joystick(64, 0, 75, 0);
    pbf_move_left_joystick(0, 128, 115, 0);
    pbf_move_left_joystick(72, 255, 50, 0);
    pbf_move_left_joystick(128, 0, 110, 0);
    pbf_mash_button(BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(192, 0, 200, 0);
}
void regidrago(void){
    pbf_move_left_joystick(16, 255, 160, 0);
    pbf_move_left_joystick(72, 255, 50, 0);
    pbf_move_left_joystick(48, 255, 104, 0);
    pbf_move_left_joystick(128, 0, 48, 0);
    pbf_move_left_joystick(0, 56, 60, 0);
    pbf_move_left_joystick(64, 0, 60, 0);
    pbf_move_left_joystick(255, 148, 75, 0);
    pbf_mash_button(BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(128, 0, 180, 0);
}



void ShinyHuntUnattendedRegi::program(SingleSwitchProgramEnvironment& env) const{
//    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);

    uint32_t last_touch = system_clock() - TOUCH_DATE_INTERVAL;
    uint16_t correct_count = 0;
    for (uint32_t c = 0; ; c++){
        //  Auto-correction.
        if (CORRECTION_INTERVAL > 0 && correct_count >= CORRECTION_INTERVAL){
            env.logger.log("Performing auto-correction.");
            //  Move down to building exit and exit.
            pbf_move_left_joystick(128, 255, 500, TRANSITION_DELAY);
            pbf_move_left_joystick(128, 255, 300, TRANSITION_DELAY);

            //  Navigate back into the corner.
            pbf_move_left_joystick(255, 64, 200, 0);
            pbf_move_left_joystick(120, 0, 250, 0);
            pbf_move_left_joystick(255, 100, 150, 10);
            correct_count = 0;
        }else{
            //  Move to corner.
            pbf_move_left_joystick(255, 100, 300, 10);
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
        switch (REGI_NAME){
        case RegiGolem::Regirock:
            regirock();
            env.logger.log("Starting Regirock Encounter: " + tostr_u_commas(c + 1));
            break;
        case RegiGolem::Regice:
            regice();
            env.logger.log("Starting Regice Encounter: " + tostr_u_commas(c + 1));
            break;
        case RegiGolem::Registeel:
            registeel();
            env.logger.log("Starting Registeel Encounter: " + tostr_u_commas(c + 1));
            break;
        case RegiGolem::Regieleki:
            regieleki();
            env.logger.log("Starting Regieleki Encounter: " + tostr_u_commas(c + 1));
            break;
        case RegiGolem::Regidrago:
            regidrago();
            env.logger.log("Starting Regidrago Encounter: " + tostr_u_commas(c + 1));
            break;
        }

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



