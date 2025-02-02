/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_MassRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


MassRelease_Descriptor::MassRelease_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:MassRelease",
        STRING_POKEMON + " SwSh", "Mass Release",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MassRelease.md",
        "Mass release boxes of " + STRING_POKEMON + ".",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}



MassRelease::MassRelease()
    : BOXES_TO_RELEASE(
        "<b>Number of Boxes to Release:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        2, 0, 32
    )
    , DODGE_SYSTEM_UPDATE_WINDOW(
        "<b>Dodge System Update Window:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(BOXES_TO_RELEASE);
    PA_ADD_OPTION(DODGE_SYSTEM_UPDATE_WINDOW);
}

void MassRelease::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, DODGE_SYSTEM_UPDATE_WINDOW);
    }else{
        pbf_press_button(context, BUTTON_LCLICK, 5, 5);
    }

    release_boxes(
        context,
        BOXES_TO_RELEASE,
        GameSettings::instance().BOX_SCROLL_DELAY0,
        GameSettings::instance().BOX_CHANGE_DELAY0
    );
    pbf_press_button(context, BUTTON_HOME, 80ms, GameSettings::instance().HOME_TO_GAME_DELAY0);
}



}
}
}

