/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh_FriendSearchDisconnect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


FriendSearchDisconnect_Descriptor::FriendSearchDisconnect_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:FriendSearchDisconnect",
        STRING_POKEMON + " SwSh", "Friend Search Disconnect",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/FriendSearchDisconnect.md",
        "Disconnect from the internet using the friend search method.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}



FriendSearchDisconnect::FriendSearchDisconnect()
    : USER_SLOT(
        "<b>User Slot:</b><br>Use this profile to disconnect.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 8
    )
{
    PA_ADD_OPTION(USER_SLOT);
}

void FriendSearchDisconnect::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);

    home_to_add_friends(context, USER_SLOT - 1, 1, true);

    //  Enter friend search.
    pbf_mash_button(context, BUTTON_A, 100);
    settings_to_enter_game(context, true);
}


}
}
}
