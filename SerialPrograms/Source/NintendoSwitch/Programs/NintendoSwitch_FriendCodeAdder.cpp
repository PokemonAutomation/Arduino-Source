/*  Friend Code Adder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/FixedInterval.h"
#include "NintendoSwitch_FriendCodeAdder.h"
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
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


FriendCodeAdder::FriendCodeAdder()
    : USER_SLOT(
        "<b>User Slot:</b><br>Send friend requests for this profile.",
        LockWhileRunning::LOCKED,
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
    , OPEN_CODE_PAD_DELAY(
        "<b>Open Code Pad Delay</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "1 * TICKS_PER_SECOND"
    )
    , SEARCH_TIME(
        "<b>Search Time:</b><br>Wait this long after initiating search.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , TOGGLE_BEST_STATUS_DELAY(
        "<b>Toggle Best Delay:</b><br>Time needed to toggle the best friend status.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "1 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(USER_SLOT);
    PA_ADD_OPTION(FRIEND_CODES);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(OPEN_CODE_PAD_DELAY);
    PA_ADD_OPTION(SEARCH_TIME);
    PA_ADD_OPTION(TOGGLE_BEST_STATUS_DELAY);
}

void FriendCodeAdder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    grip_menu_connect_go_home(context);

    bool first = true;
    for (const std::string& line : FRIEND_CODES.lines()){
        std::vector<uint8_t> code = FriendCodeListOption::parse(line);
        if (code.size() != 12){
            continue;
        }

        PokemonSwSh::home_to_add_friends(context, USER_SLOT - 1, 3, first);
        first = false;

        ssf_press_button1(context, BUTTON_A, OPEN_CODE_PAD_DELAY);
        enter_digits(context, 12, &code[0]);

        pbf_wait(context, SEARCH_TIME);
        ssf_press_button1(context, BUTTON_A, TOGGLE_BEST_STATUS_DELAY);
        ssf_press_button1(context, BUTTON_A, TOGGLE_BEST_STATUS_DELAY);
        pbf_press_button(context, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
    }
}



}
}
