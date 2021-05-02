/*  Friend Code Adder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "NintendoSwitch/FixedInterval.h"
#include "FriendCodeAdder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


FriendCodeAdder::FriendCodeAdder()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Friend Code Adder",
        "SerialPrograms/FriendCodeAdder.md",
        "Add a list of friend codes."
    )
    , USER_SLOT(
        "<b>User Slot:</b><br>Send friend requests for this profile.",
        1, 1, 8
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , OPEN_CODE_PAD_DELAY(
        "<b>Open Code Pad Delay</b>",
        "1 * TICKS_PER_SECOND"
    )
    , SEARCH_TIME(
        "<b>Search Time:</b><br>Wait this long after initiating search.",
        "3 * TICKS_PER_SECOND"
    )
    , TOGGLE_BEST_STATUS_DELAY(
        "<b>Toggle Best Delay:</b><br>Time needed to toggle the best friend status.",
        "1 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&USER_SLOT, "USER_SLOT");
    m_options.emplace_back(&FRIEND_CODES, "FRIEND_CODES");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&OPEN_CODE_PAD_DELAY, "OPEN_CODE_PAD_DELAY");
    m_options.emplace_back(&SEARCH_TIME, "SEARCH_TIME");
    m_options.emplace_back(&TOGGLE_BEST_STATUS_DELAY, "TOGGLE_BEST_STATUS_DELAY");
}

void FriendCodeAdder::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);

    bool first = true;
    for (const QString& line : FRIEND_CODES.lines()){
        std::vector<uint8_t> code = FriendCodeList::parse(line);
        if (code.size() != 12){
            continue;
        }

        home_to_add_friends(env.console, USER_SLOT - 1, 3, first);
        first = false;

        ssf_press_button1(env.console, BUTTON_A, OPEN_CODE_PAD_DELAY);
        enter_digits(env.console, 12, &code[0]);

        pbf_wait(env.console, SEARCH_TIME);
        ssf_press_button1(env.console, BUTTON_A, TOGGLE_BEST_STATUS_DELAY);
        ssf_press_button1(env.console, BUTTON_A, TOGGLE_BEST_STATUS_DELAY);
        pbf_press_button(env.console, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
