/*  ShinyHuntUnattended-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-Regigigas2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntUnattendedRegigigas2::ShinyHuntUnattendedRegigigas2()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Unattended - Regigigas2",
        "NativePrograms/ShinyHuntUnattended-Regigigas2.md",
        "A new version of the Regigigas program that is faster."
    )
    , REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        24
    )
    , START_TO_ATTACK_DELAY(
        "<b>Start to Attack Delay:</b><br>This needs to be carefully calibrated.",
        "3750"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ATTACK_TO_CATCH_DELAY(
        "<b>Attack to Catch Delay:</b><br>Increase this if you seem to be catching Regigigas very often.",
        "9 * TICKS_PER_SECOND"
    )
    , CATCH_TO_OVERWORLD_DELAY(
        "<b>Catch to Overworld Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&REVERSAL_PP, "REVERSAL_PP");
    m_options.emplace_back(&START_TO_ATTACK_DELAY, "START_TO_ATTACK_DELAY");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&ATTACK_TO_CATCH_DELAY, "ATTACK_TO_CATCH_DELAY");
    m_options.emplace_back(&CATCH_TO_OVERWORLD_DELAY, "CATCH_TO_OVERWORLD_DELAY");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
}

void ShinyHuntUnattendedRegigigas2::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    uint32_t last_touch = system_clock();
    if (TOUCH_DATE_INTERVAL > 0){
        touch_date_from_home(SETTINGS_TO_HOME_DELAY);
    }

    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);

    uint32_t encounter = 0;
    while (true){
        for (uint8_t pp = REVERSAL_PP; pp > 0; pp--){
            env.logger.log("Starting Regigigas Encounter: " + tostr_u_commas(++encounter));

            pbf_press_button(BUTTON_A, 10, 3 * TICKS_PER_SECOND);
            pbf_press_button(BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_press_button(BUTTON_A, 10, START_TO_ATTACK_DELAY);

            set_leds(true);
            pbf_press_button(BUTTON_A, 10, 2 * TICKS_PER_SECOND);
            set_leds(false);

            //  Enter Pokemon menu if shiny.
            pbf_press_dpad(DPAD_DOWN, 10, 0);
            pbf_mash_button(BUTTON_A, 2 * TICKS_PER_SECOND);

            pbf_press_dpad(DPAD_DOWN, 10, 0);
            pbf_press_button(BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_press_dpad(DPAD_DOWN, 10, 0);
            pbf_press_button(BUTTON_A, 10, TICKS_PER_SECOND);

            pbf_wait(ATTACK_TO_CATCH_DELAY);
            pbf_press_dpad(DPAD_DOWN, 10, 0);
            pbf_press_button(BUTTON_A, 10, CATCH_TO_OVERWORLD_DELAY);
        }

        //  Touch the date and conditional close game.
        if (TOUCH_DATE_INTERVAL > 0 && system_clock() - last_touch >= TOUCH_DATE_INTERVAL){
            last_touch += TOUCH_DATE_INTERVAL;
            close_game_if_overworld(true, 0);
        }else{
            close_game_if_overworld(false, 0);
        }

        start_game_from_home(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 0, 0, false);
    }

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
}



}
}
}
