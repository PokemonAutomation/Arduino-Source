/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "PokemonSwSh_DateSpam-LotoFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

LotoFarmer::LotoFarmer()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Date Spam: Loto Farmer",
        "NativePrograms/DateSpam-LotoFarmer.md",
        "Farm the Loto ID."
    )
    , SKIPS(
        "<b>Number of Loto Attempts:</b>",
        100000
    )
    , MASH_B_DURATION(
        "<b>Mash B for this long to exit the dialog:</b><br>(Some languages like German need to increase this.)",
        "8 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&SKIPS, "SKIPS");
    m_options.emplace_back(&MASH_B_DURATION, "MASH_B_DURATION");
}

void LotoFarmer::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    uint8_t year = MAX_YEAR;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.logger.log("Fetch Attempts: " + tostr_u_commas(c));
        home_roll_date_enter_game_autorollback(&year);
        pbf_mash_button(BUTTON_B, 90);

        pbf_press_button(BUTTON_A, 10, 70);
        pbf_press_button(BUTTON_B, 10, 70);
        pbf_press_dpad(DPAD_DOWN, 10, 5);
        pbf_mash_button(BUTTON_ZL, 490);
        pbf_mash_button(BUTTON_B, MASH_B_DURATION);

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        pbf_press_button(BUTTON_HOME, 10, 5);
        pbf_mash_button(BUTTON_B, GAME_TO_HOME_DELAY_FAST - 15);
    }

    end_program_callback();
    end_program_loop();
}


}
}
}
