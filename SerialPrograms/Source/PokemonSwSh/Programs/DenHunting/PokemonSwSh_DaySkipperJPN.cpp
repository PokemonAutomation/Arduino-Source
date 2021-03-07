/*  Day Skipper (JPN)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDaySkippers.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh_DaySkipperJPN.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

DaySkipperJPN::DaySkipperJPN()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_31KB,
        "Day Skipper (JPN)",
        "NativePrograms/DaySkipperJPN.md",
        "A day skipper for Japanese date format. (7600 skips/hour)"
    )
    , SKIPS(
        "<b>Number of Frame Skips:</b>",
        10
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CORRECTION_SKIPS(
        "<b>Auto-Correct Interval:</b><br>Run auto-recovery every this # of skips. Zero disables the auto-corrections.",
        1000
    )
{
    m_options.emplace_back(&SKIPS, "SKIPS");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&CORRECTION_SKIPS, "CORRECTION_SKIPS");
}

void DaySkipperJPN::program(SingleSwitchProgramEnvironment& env) const{
    //  Setup globals.
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(BUTTON_ZR, 5, 5);

    //  Setup starting state.
    skipper_init_view();

    uint8_t day = 1;
    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        skipper_increment_day(false);

        if (day == 31){
            day = 1;
        }else{
            correct_count++;
            day++;
            remaining_skips--;
            env.logger.log("Skips Remaining: " + tostr_u_commas(remaining_skips));
        }
        if (CORRECTION_SKIPS != 0 && correct_count == CORRECTION_SKIPS){
            correct_count = 0;
            skipper_auto_recovery();
        }
    }

    //  Prevent the Switch from sleeping and the time from advancing.
    end_program_callback();
    pbf_wait(15 * TICKS_PER_SECOND);
    while (true){
        ssf_press_button1(BUTTON_A, 15 * TICKS_PER_SECOND);
    }
}


}
}
}
