/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_MassRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


MassRelease_Descriptor::MassRelease_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:MassRelease",
        STRING_POKEMON + " SwSh", "Mass Release",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MassRelease.md",
        "Mass release boxes of " + STRING_POKEMON + ".",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



MassRelease::MassRelease(const MassRelease_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , BOXES_TO_RELEASE(
        "<b>Number of Boxes to Release:</b>",
        2, 0, 32
    )
    , DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        false
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(BOXES_TO_RELEASE);
    PA_ADD_OPTION(DODGE_SYSTEM_UPDATE_WINDOW);
}

void MassRelease::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);
    }

    release_boxes(
        env.console,
        BOXES_TO_RELEASE,
        GameSettings::instance().BOX_SCROLL_DELAY,
        GameSettings::instance().BOX_CHANGE_DELAY
    );
    pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().HOME_TO_GAME_DELAY);
}



}
}
}

