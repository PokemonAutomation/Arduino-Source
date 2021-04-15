/*  Day Skipper (JPN) - 7.8k version
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDaySkippers.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh_DaySkipperJPN-7.8k.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

DaySkipperJPN7p8k::DaySkipperJPN7p8k()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_31KB,
        "Day Skipper (JPN) - 7.8k",
        "NativePrograms/DaySkipperJPN-7.8k.md",
        "A faster, but less reliable Japanese date skipper. (7800 skips/hour)"
    )
    , SKIPS(
        "<b>Number of Frame Skips:</b>",
        10
    )
    , START_DATE(
        "<b>Start Date:</b>",
        QDate(2000, 1, 1)
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
    m_options.emplace_back(&START_DATE, "START_DATE");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&CORRECTION_SKIPS, "CORRECTION_SKIPS");
}

const uint8_t DAYS_PER_MONTH[] = {
    31, //  January
    28, //  February
    31, //  March
    30, //  April
    31, //  May
    30, //  June
    31, //  July
    31, //  August
    30, //  September
    31, //  October
    30, //  November
    31, //  December
};
uint8_t days_in_month(uint8_t year, uint8_t month){
    return month != 2
        ? DAYS_PER_MONTH[month - 1]
        : (year & 0x3) ? 28 : 29;
}
typedef struct{
    uint8_t year;
    uint8_t month;
    uint8_t day;
} DateSmall;

bool is_start(const DateSmall* date){
    return date->year != 0 || date->month != 1 || date->day != 1;
}
bool date_increment_day(DateSmall* date, bool press){
    uint8_t days = days_in_month(date->year, date->month);
    if (date->day != days){
        if (press){
            skipper_increment_day(false);
        }
        date->day++;
        return true;
    }

    date->day = 1;
    date->month++;
    if (date->month > 12){
        date->month = 1;
        date->year++;
    }
    if (date->year > 60){
        date->year = 0;
    }

    if (!press){
        return is_start(date);
    }

    if (date->month != 1){
        skipper_increment_month(DAYS_PER_MONTH[date->month - 1]);
        return true;
    }

    if (date->year != 0){
        skipper_increment_all();
        return true;
    }

    skipper_increment_all_rollback();
    return false;
}

void DaySkipperJPN7p8k::program(SingleSwitchProgramEnvironment& env) const{
    //  Setup globals.
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(BUTTON_ZL, 5, 5);

    //  Sanitize starting date.
    uint16_t year = (uint16_t)((QDate)START_DATE).year();
    if (year < 2000) year = 2000;
    if (year > 2060) year = 2060;

    DateSmall date;
    date.year   = (uint8_t)(year - 2000);
    date.month  = (uint8_t)((QDate)START_DATE).month();
    date.day    = (uint8_t)((QDate)START_DATE).day();
    if (date.month < 1)     date.month = 1;
    if (date.month > 12)    date.month = 12;
    if (date.day < 1)       date.day = 1;
    {
        uint8_t day = days_in_month(date.year, date.month);
        if (date.day > day) date.day = day;
    }

    //  Setup starting state.
    skipper_init_view();

    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        if (date_increment_day(&date, true)){
            remaining_skips--;
            correct_count++;
            env.log("Expected Date: " + QDate(date.year + 2000, date.month, date.day).toString("yyyy/MM/dd"));
            env.log("Skips Remaining: " + tostr_u_commas(remaining_skips));
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



