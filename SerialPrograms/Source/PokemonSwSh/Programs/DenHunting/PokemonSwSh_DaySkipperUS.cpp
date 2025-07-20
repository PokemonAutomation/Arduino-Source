/*  Day Skipper (US)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_OFFICIAL

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_DateSkippers.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_DaySkipperStats.h"
#include "PokemonSwSh_DaySkipperUS.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


DaySkipperUS_Descriptor::DaySkipperUS_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DaySkipperUS",
        STRING_POKEMON + " SwSh", "Day Skipper (US)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DaySkipperUS.md",
        "A day skipper for US date format that.  (Switch 1: ~7100 skips/hour, Switch 2: 5443 skips/hour)",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {
            ControllerFeature::TickPrecise,
            ControllerFeature::NintendoSwitch_ProController,
            // ControllerFeature::NintendoSwitch_DateSkip,
        }
    )
{}
std::unique_ptr<StatsTracker> DaySkipperUS_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new SkipperStats());
}



DaySkipperUS::DaySkipperUS()
    : SKIPS(
        "<b>Number of Frame Skips:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        10
    )
    , REAL_LIFE_YEAR(
        "<b>Real Life Year:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        std::min(current_year(), (uint16_t)2060),
        2000, 2060
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
    PA_ADD_OPTION(REAL_LIFE_YEAR);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CORRECTION_SKIPS);
}



void DaySkipperUS::run_switch1(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    using namespace DateSkippers::Switch1;

    bool needs_inference = context->performance_class() != ControllerPerformanceClass::SerialPABotBase_Wired;

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
    init_view(context);
    rollback_year_full(context, true);
    year = 0;

    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        send_program_status_notification(env, NOTIFICATION_PROGRESS_UPDATE);

        if (needs_inference){
            increment_day_with_feedback(env.console, context, true);
        }else{
            increment_day(context, true);
        }


        correct_count++;
        year++;
        remaining_skips--;
        stats.issued++;
//        env.log("Skips Remaining: " + tostr_u_commas(remaining_skips));
        env.update_stats();

        if (year >= 60){
            if (real_life_year <= 36){
                rollback_year_sync(context);
                year = real_life_year;
            }else{
                rollback_year_full(context, true);
                year = 0;
            }
        }
        if (CORRECTION_SKIPS != 0 && correct_count == CORRECTION_SKIPS){
            correct_count = 0;
            auto_recovery(context);
        }
    }

    //  Prevent the Switch from sleeping and the time from advancing.
    context.wait_for_all_requests();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

    pbf_wait(context, 15 * TICKS_PER_SECOND);
    while (true){
        ssf_press_button(context, BUTTON_A, 15000ms);
    }
}
void DaySkipperUS::run_switch2(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    using namespace DateSkippers::Switch2;

    if (context->performance_class() != ControllerPerformanceClass::SerialPABotBase_Wired){
        throw UserSetupError(
            env.logger(),
            "This program requires a tick precise wired controller."
        );
    }

    SkipperStats& stats = env.current_stats<SkipperStats>();
    stats.total_skips = SKIPS;
    stats.runs++;

    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(context, BUTTON_ZR, 5, 5);

    //  Setup starting state.
    init_view(context);

    uint8_t day = 1;
    while (remaining_skips > 0){
        send_program_status_notification(env, NOTIFICATION_PROGRESS_UPDATE);

        increment_day_us(context);

        if (day == 31){
            day = 1;
        }else{
            day++;
            remaining_skips--;
            stats.issued++;
            env.update_stats();
        }
    }

    //  Prevent the Switch from sleeping and the time from advancing.
    context.wait_for_all_requests();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

    while (true){
        ssf_press_button(context, BUTTON_A, 760ms);
        for (int c = 0; c < 10; c++){
            ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
        }
        ssf_press_button(context, BUTTON_A, 14000ms, 80ms);
    }
}





void DaySkipperUS::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ConsoleType console_type = env.console.state().console_type();
    if (is_switch1(console_type)){
        run_switch1(env, context);
        return;
    }
    if (is_switch2(console_type)){
        run_switch2(env, context);
        return;
    }
    throw UserSetupError(
        env.console,
        "Please select a valid Switch console type."
    );
}



}
}
}
#endif
