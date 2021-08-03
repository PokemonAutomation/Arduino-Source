/*  Day Skipper (EU)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDaySkippers.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh_DaySkipperStats.h"
#include "PokemonSwSh_DaySkipperEU.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


DaySkipperEU_Descriptor::DaySkipperEU_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:DaySkipperEU",
        "Day Skipper (EU)",
        "SwSh-Arduino/wiki/Basic:-DaySkipperEU",
        "A day skipper for EU date format that.  (~7500 skips/hour)",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



DaySkipperEU::DaySkipperEU(const DaySkipperEU_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SKIPS(
        "<b>Number of Frame Skips:</b>",
        10
    )
    , REAL_LIFE_YEAR(
        "<b>Real Life Year:</b>",
        2021, 2000, 2060
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
    m_options.emplace_back(&REAL_LIFE_YEAR, "REAL_LIFE_YEAR");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&CORRECTION_SKIPS, "CORRECTION_SKIPS");
}

std::unique_ptr<StatsTracker> DaySkipperEU::make_stats() const{
    return std::unique_ptr<StatsTracker>(new SkipperStats());
}

void DaySkipperEU::program(SingleSwitchProgramEnvironment& env){
    SkipperStats& stats = env.stats<SkipperStats>();
    stats.runs++;

    //  Setup globals.
    uint8_t real_life_year = (uint8_t)(
        REAL_LIFE_YEAR < 2000 ?  0 :
        REAL_LIFE_YEAR > 2060 ? 60 : REAL_LIFE_YEAR - 2000
    );
    uint8_t year = 60;
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(env.console, BUTTON_ZR, 5, 5);

    //  Setup starting state.
    skipper_init_view(env.console);
    skipper_rollback_year_full(env.console, false);
    year = 0;

    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        skipper_increment_day(env.console, false);

        correct_count++;
        year++;
        remaining_skips--;
        stats.issued++;
//        env.log("Skips Remaining: " + tostr_u_commas(remaining_skips));
        env.update_stats(stats.to_str_current(remaining_skips));

        if (year >= 60){
            if (real_life_year <= 36){
                skipper_rollback_year_sync(env.console);
                year = real_life_year;
            }else{
                skipper_rollback_year_full(env.console, false);
                year = 0;
            }
        }
        if (CORRECTION_SKIPS != 0 && correct_count == CORRECTION_SKIPS){
            correct_count = 0;
            skipper_auto_recovery(env.console);
        }
    }

    //  Prevent the Switch from sleeping and the time from advancing.
    end_program_callback(env.console);
    pbf_wait(env.console, 15 * TICKS_PER_SECOND);
    while (true){
        ssf_press_button1(env.console, BUTTON_A, 15 * TICKS_PER_SECOND);
    }
}


}
}
}
