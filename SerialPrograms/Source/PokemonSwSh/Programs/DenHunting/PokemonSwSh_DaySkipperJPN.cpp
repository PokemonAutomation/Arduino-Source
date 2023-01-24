/*  Day Skipper (JPN)
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
#include "PokemonSwSh_DaySkipperJPN.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


DaySkipperJPN_Descriptor::DaySkipperJPN_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DaySkipperJPN",
        STRING_POKEMON + " SwSh", "Day Skipper (JPN)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DaySkipperJPN.md",
        "A day skipper for Japanese date format. (7600 skips/hour)",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}

std::unique_ptr<StatsTracker> DaySkipperJPN_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new SkipperStats());
}



DaySkipperJPN::DaySkipperJPN()
    : SKIPS(
        "<b>Number of Frame Skips:</b>",
        LockWhileRunning::LOCKED,
        10
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
        LockWhileRunning::LOCKED,
        1000
    )
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CORRECTION_SKIPS);
}

void DaySkipperJPN::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    SkipperStats& stats = env.current_stats<SkipperStats>();
    stats.total_skips = SKIPS;
    stats.runs++;

    //  Setup globals.
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(context, BUTTON_ZR, 5, 5);

    //  Setup starting state.
    skipper_init_view(context);

    uint8_t day = 1;
    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        send_program_status_notification(env, NOTIFICATION_PROGRESS_UPDATE);

        skipper_increment_day(context, false);

        if (day == 31){
            day = 1;
        }else{
            correct_count++;
            day++;
            remaining_skips--;
            stats.issued++;
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
