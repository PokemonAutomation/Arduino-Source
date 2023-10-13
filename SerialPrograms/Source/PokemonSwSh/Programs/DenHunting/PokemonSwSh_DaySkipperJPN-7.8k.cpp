/*  Day Skipper (JPN) - 7.8k version
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/FixedInterval.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DaySkippers.h"
#include "PokemonSwSh_DaySkipperStats.h"
#include "PokemonSwSh_DaySkipperJPN-7.8k.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


DaySkipperJPN7p8k_Descriptor::DaySkipperJPN7p8k_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DaySkipperJPN7p8k",
        STRING_POKEMON + " SwSh", "Day Skipper (JPN) - 7.8k",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DaySkipperJPN-7.8k.md",
        "A faster, but less reliable Japanese date skipper. (7800 skips/hour)",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}
std::unique_ptr<StatsTracker> DaySkipperJPN7p8k_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new SkipperStats());
}



DaySkipperJPN7p8k::DaySkipperJPN7p8k()
    : SKIPS(
        "<b>Number of Frame Skips:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        10
    )
    , START_DATE(
        "<b>Start Date:</b>",
        QDate(2000, 1, 1), QDate(2060, 12, 31),
        QDate(2000, 1, 1)
    )
    , NOTIFICATION_PROGRESS_UPDATE("Progress Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRESS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CORRECTION_SKIPS(
        "<b>Auto-Correct Interval:</b><br>Run auto-recovery every this # of skips. Zero disables the auto-corrections.",
        LockMode::LOCK_WHILE_RUNNING,
        1000
    )
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(START_DATE);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CORRECTION_SKIPS);
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
bool date_increment_day(BotBaseContext& context, DateSmall* date, bool press){
    uint8_t days = days_in_month(date->year, date->month);
    if (date->day != days){
        if (press){
            skipper_increment_day(context, false);
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
        skipper_increment_month(context, DAYS_PER_MONTH[date->month - 1]);
        return true;
    }

    if (date->year != 0){
        skipper_increment_all(context);
        return true;
    }

    skipper_increment_all_rollback(context);
    return false;
}

void DaySkipperJPN7p8k::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    SkipperStats& stats = env.current_stats<SkipperStats>();
    stats.total_skips = SKIPS;
    stats.runs++;

    //  Setup globals.
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(context, BUTTON_ZL, 5, 5);

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
    skipper_init_view(context);

    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        send_program_status_notification(env, NOTIFICATION_PROGRESS_UPDATE);

        if (date_increment_day(context, &date, true)){
            correct_count++;
            remaining_skips--;
            stats.issued++;
            env.log("Expected Date: " + QDate(date.year + 2000, date.month, date.day).toString("yyyy/MM/dd").toStdString());
//            env.log("Skips Remaining: " + tostr_u_commas(remaining_skips));
            env.update_stats();
        }
        if (CORRECTION_SKIPS != 0 && correct_count == CORRECTION_SKIPS){
            correct_count = 0;
            skipper_auto_recovery(context);
        }

    }

    //  Prevent the Switch from sleeping and the time from advancing.
    context.wait_for_all_requests();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

    pbf_wait(context, 15 * TICKS_PER_SECOND);
    while (true){
        ssf_press_button1(context, BUTTON_A, 15 * TICKS_PER_SECOND);
    }
}



}
}
}



