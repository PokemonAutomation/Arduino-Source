/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_MassRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


MassRelease_Descriptor::MassRelease_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:MassRelease",
        "Mass Release",
        "SwSh-Arduino/wiki/Basic:-MassRelease",
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

    release_boxes(env.console, BOXES_TO_RELEASE, BOX_SCROLL_DELAY, BOX_CHANGE_DELAY);
    pbf_press_button(env.console, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

