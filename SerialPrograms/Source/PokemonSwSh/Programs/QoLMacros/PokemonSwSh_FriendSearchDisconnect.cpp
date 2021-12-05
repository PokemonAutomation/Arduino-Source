/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh_FriendSearchDisconnect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


FriendSearchDisconnect_Descriptor::FriendSearchDisconnect_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:FriendSearchDisconnect",
        STRING_POKEMON + " SwSh", "Friend Search Disconnect",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/FriendSearchDisconnect.md",
        "Disconnect from the internet using the friend search method.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



FriendSearchDisconnect::FriendSearchDisconnect(const FriendSearchDisconnect_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , USER_SLOT(
        "<b>User Slot:</b><br>Use this profile to disconnect.",
        1, 1, 8
    )
{
    PA_ADD_OPTION(USER_SLOT);
}

void FriendSearchDisconnect::program(SingleSwitchProgramEnvironment& env) {
    ssf_press_button2(env.console, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE, 10);

    home_to_add_friends(env.console, USER_SLOT - 1, 1, true);

    //  Enter friend search.
    pbf_mash_button(env.console, BUTTON_A, 100);
    settings_to_enter_game(env.console, true);
}


}
}
}
