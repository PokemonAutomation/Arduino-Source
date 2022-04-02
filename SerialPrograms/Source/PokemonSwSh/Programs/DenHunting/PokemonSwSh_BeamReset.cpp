/*  Beam Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_BeamReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BeamReset_Descriptor::BeamReset_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:BeamReset",
        STRING_POKEMON + " SwSh", "Beam Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/BeamReset.md",
        "Reset a beam until you see a purple beam.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BeamReset::BeamReset(const BeamReset_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , DELAY_BEFORE_RESET(
        "<b>Delay before Reset:</b>",
        "5 * TICKS_PER_SECOND"
    )
    , EXTRA_LINE(
        "<b>Extra Line:</b><br>(German has an extra line of text.)",
        false
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(DELAY_BEFORE_RESET);
    PA_ADD_OPTION(EXTRA_LINE);
}

void BeamReset::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_front_of_den_nowatts(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    while (true){
        //  Talk to den.
        pbf_press_button(env.console, BUTTON_A, 10, 450);
        if (EXTRA_LINE){
            pbf_press_button(env.console, BUTTON_A, 10, 300);
        }
        pbf_press_button(env.console, BUTTON_A, 10, 300);

        //  Drop wishing piece.
        pbf_press_button(env.console, BUTTON_A, 10, 70);
        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);

        for (uint16_t c = 0; c < 4; c++){
            pbf_press_button(env.console, BUTTON_HOME, 10, 10);
            pbf_press_button(env.console, BUTTON_HOME, 10, 220);
        }
        pbf_wait(env.console, DELAY_BEFORE_RESET);

        reset_game_from_home(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }
}



}
}
}
