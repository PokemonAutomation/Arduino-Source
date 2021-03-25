/*  Beam Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_BeamReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

BeamReset::BeamReset()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Beam Reset",
        "NativePrograms/BeamReset.md",
        "Reset a beam until you see a purple beam."
    )
    , DELAY_BEFORE_RESET(
        "<b>Delay before Reset:</b>",
        "5 * TICKS_PER_SECOND"
    )
    , EXTRA_LINE(
        "<b>Extra Line:</b><br>(German has an extra line of text.)",
        false
    )
{
    m_options.emplace_back(&DELAY_BEFORE_RESET, "DELAY_BEFORE_RESET");
    m_options.emplace_back(&EXTRA_LINE, "EXTRA_LINE");
}

void BeamReset::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    resume_game_front_of_den_nowatts(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    pbf_mash_button(BUTTON_B, 100);

    while (true){
        //  Talk to den.
        pbf_press_button(BUTTON_A, 10, 450);
        if (EXTRA_LINE){
            pbf_press_button(BUTTON_A, 10, 300);
        }
        pbf_press_button(BUTTON_A, 10, 300);

        //  Drop wishing piece.
        pbf_press_button(BUTTON_A, 10, 70);
        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);

        for (uint16_t c = 0; c < 4; c++){
            pbf_press_button(BUTTON_HOME, 10, 10);
            pbf_press_button(BUTTON_HOME, 10, 220);
        }
        pbf_wait(DELAY_BEFORE_RESET);

        reset_game_from_home(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }
}



}
}
}
