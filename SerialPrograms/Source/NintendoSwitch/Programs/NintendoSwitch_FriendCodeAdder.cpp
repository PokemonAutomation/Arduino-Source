/*  Friend Code Adder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_FriendCodeAdder.h"
#include "NintendoSwitch/Programs/NintendoSwitch_NumberCodeEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


FriendCodeAdder_Descriptor::FriendCodeAdder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:FriendCodeAdder",
        "Nintendo Switch", "Friend Code Adder",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/FriendCodeAdder.md",
        "Add a list of friend codes.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController},
        FasterIfTickPrecise::MUCH_FASTER
    )
{}


FriendCodeAdder::FriendCodeAdder()
    : USER_SLOT(
        "<b>User Slot:</b><br>Send friend requests for this profile.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 8
    )
    , FRIEND_CODES(
        "<b>Friend Codes:</b> One per line only. Invalid characters are ignored.",
        {
            "SW-1234-5678-9012",
            "123456789012",
        }
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , OPEN_CODE_PAD_DELAY0(
        "<b>Open Code Pad Delay</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , SEARCH_TIME0(
        "<b>Search Time:</b><br>Wait this long after initiating search.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , TOGGLE_BEST_STATUS_DELAY0(
        "<b>Toggle Best Delay:</b><br>Time needed to toggle the best friend status.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
{
    PA_ADD_OPTION(USER_SLOT);
    PA_ADD_OPTION(FRIEND_CODES);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(OPEN_CODE_PAD_DELAY0);
    PA_ADD_OPTION(SEARCH_TIME0);
    PA_ADD_OPTION(TOGGLE_BEST_STATUS_DELAY0);
}

void FriendCodeAdder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    grip_menu_connect_go_home(context);

    bool first = true;
    for (const std::string& line : FRIEND_CODES.lines()){
        std::string code = FriendCodeListOption::parse(line);
        if (code.size() != 12){
            continue;
        }

        PokemonSwSh::home_to_add_friends(context, USER_SLOT - 1, 3, first);
        first = false;

        ssf_press_button_ptv(context, BUTTON_A, OPEN_CODE_PAD_DELAY0);
        numberpad_enter_code(env.logger(), context, code, true);

        pbf_wait(context, SEARCH_TIME0);
        ssf_press_button(context, BUTTON_A, TOGGLE_BEST_STATUS_DELAY0);
        ssf_press_button(context, BUTTON_A, TOGGLE_BEST_STATUS_DELAY0);
        pbf_press_button(context, BUTTON_HOME, 80ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    }
}



}
}
