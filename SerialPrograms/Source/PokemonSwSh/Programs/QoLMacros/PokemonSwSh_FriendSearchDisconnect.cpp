/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh_FriendSearchDisconnect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

FriendSearchDisconnect::FriendSearchDisconnect()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Friend Search Disconnect",
        "SerialPrograms/FriendSearchDisconnect.md",
        "Disconnect from the internet using the friend search method."
    )
    , USER_SLOT(
        "<b>User Slot:</b><br>Use this profile to disconnect.",
        1, 1, 8
    )
{
    m_options.emplace_back(&USER_SLOT, "USER_SLOT");
}

void FriendSearchDisconnect::program(SingleSwitchProgramEnvironment& env) const{
    ssf_press_button2(env.console, BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);

    home_to_add_friends(env.console, USER_SLOT - 1, 1, true);

    //  Enter friend search.
    pbf_mash_button(env.console, BUTTON_A, 100);
    settings_to_enter_game(env.console, true);
}


}
}
}
