/*  Day Skipper (EU)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/FixedInterval.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DaySkippers.h"
#include "PokemonSwSh_DaySkipperStats.h"
#include "PokemonSwSh_DaySkipperEU.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


DaySkipperEU_Descriptor::DaySkipperEU_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DaySkipperEU",
        STRING_POKEMON + " SwSh", "Day Skipper (EU)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DaySkipperEU.md",
        "A day skipper for EU date format that.  (~7500 skips/hour)",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}
std::unique_ptr<StatsTracker> DaySkipperEU_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new SkipperStats());
}



DaySkipperEU::DaySkipperEU()
    : SKIPS(
        "<b>Number of Frame Skips:</b>",
        LockWhileRunning::LOCKED,
        10
    )
    , REAL_LIFE_YEAR(
        "<b>Real Life Year:</b>",
        LockWhileRunning::LOCKED,
        2023, 2000, 2060
    )
    , NOTIFICATION_PROGRESS_UPDATE("Progress Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
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
        LockWhileRunning::LOCKED,
        1000
    )
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(REAL_LIFE_YEAR);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CORRECTION_SKIPS);
}


void DaySkipperEU::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    SkipperStats& stats = env.current_stats<SkipperStats>();
    stats.total_skips = SKIPS;
    stats.runs++;

    //  Setup globals.
    uint8_t real_life_year = (uint8_t)(
        REAL_LIFE_YEAR < 2000 ?  0 :
        REAL_LIFE_YEAR > 2060 ? 60 : REAL_LIFE_YEAR - 2000
    );
    uint8_t year = 60;
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(context, BUTTON_ZR, 5, 5);

    //  Setup starting state.
    skipper_init_view(context);
    skipper_rollback_year_full(context, false);
    year = 0;

    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        send_program_status_notification(env, NOTIFICATION_PROGRESS_UPDATE);

        skipper_increment_day(context, false);

        correct_count++;
        year++;
        remaining_skips--;
        stats.issued++;
//        env.log("Skips Remaining: " + tostr_u_commas(remaining_skips));
        env.update_stats();

        if (year >= 60){
            if (real_life_year <= 36){
                skipper_rollback_year_sync(context);
                year = real_life_year;
            }else{
                skipper_rollback_year_full(context, false);
                year = 0;
            }
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
