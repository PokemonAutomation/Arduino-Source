/*  ShinyHuntUnattended-SwordsOfJustice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntUnattendedSwordsOfJustice::ShinyHuntUnattendedSwordsOfJustice()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Unattended - Swords Of Justice",
        "NativePrograms/ShinyHuntUnattended-SwordsOfJustice.md",
        "Hunt for shiny SOJs. Stop when a shiny is found."
    )
    , EXIT_CAMP_TO_RUN_DELAY(
        "<b>Exit Camp to Run Delay:</b><br>This needs to be carefully calibrated.",
        "1890"
    )
    , AIRPLANE_MODE(
        "<b>Airplane Mode:</b><br>Enable if airplane mode is on.",
        false
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ENTER_CAMP_DELAY(
        "<b>Enter Camp Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
{
    m_options.emplace_back(&EXIT_CAMP_TO_RUN_DELAY, "EXIT_CAMP_TO_RUN_DELAY");
    m_options.emplace_back(&AIRPLANE_MODE, "AIRPLANE_MODE");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&ENTER_CAMP_DELAY, "ENTER_CAMP_DELAY");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
}



void ShinyHuntUnattendedSwordsOfJustice::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock();
    for (uint32_t c = 0; ; c++){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock() - last_touch >= PERIOD){
            pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        //  Trigger encounter.
        pbf_press_button(BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
        pbf_press_button(BUTTON_A, 10, ENTER_CAMP_DELAY);
        if (AIRPLANE_MODE){
            pbf_press_button(BUTTON_A, 10, 100);
            pbf_press_button(BUTTON_A, 10, 100);
        }
        pbf_press_button(BUTTON_X, 10, 50);
        pbf_press_dpad(DPAD_LEFT, 10, 10);
        env.log("Starting Encounter: " + tostr_u_commas(c + 1));
        pbf_press_button(BUTTON_A, 10, EXIT_CAMP_TO_RUN_DELAY);

        //  Run away if not shiny.
        run_away_with_lights();

        //  Enter Pokemon menu if shiny.
        enter_summary(false);
    }

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
}



}
}
}
