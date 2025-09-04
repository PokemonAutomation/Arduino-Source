/*  Day Skipper (JPN)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_OFFICIAL

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_DateSkippers.h"
#include "Pokemon/Pokemon_Strings.h"
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
        "A day skipper for Japanese date format. (Switch 1: 7600 skips/hour, Switch 2: 5655 skips/hour)",
        ProgramControllerClass::StandardController_WithRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

std::unique_ptr<StatsTracker> DaySkipperJPN_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new SkipperStats());
}



DaySkipperJPN::DaySkipperJPN()
    : SKIPS(
        "<b>Number of Frame Skips:</b>",
        LockMode::LOCK_WHILE_RUNNING,
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
        LockMode::LOCK_WHILE_RUNNING,
        1000
    )
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CORRECTION_SKIPS);
}



void DaySkipperJPN::run_switch1(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    using namespace DateSkippers::Switch1;

    bool needs_inference;
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired:
        needs_inference = false;
        break;
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
        needs_inference = true;
        break;
    default:
        throw UserSetupError(
            env.logger(),
            "This program requires a controller performance class of \"Wired\" or \"Wireless\" for the Switch 1."
        );
    }

    SkipperStats& stats = env.current_stats<SkipperStats>();
    stats.total_skips = SKIPS;
    stats.runs++;

    //  Setup globals.
    uint32_t remaining_skips = SKIPS;

    //  Connect
    pbf_press_button(context, BUTTON_ZR, 5, 5);

    //  Setup starting state.
    init_view(context);

    uint8_t day = 1;
    uint16_t correct_count = 0;
    while (remaining_skips > 0){
        send_program_status_notification(env, NOTIFICATION_PROGRESS_UPDATE);

        if (needs_inference){
            increment_day_with_feedback(env.console, context, false);
        }else{
            increment_day(context, false);
        }

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
void DaySkipperJPN::run_switch2(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    using namespace DateSkippers::Switch2;

    if (context->performance_class() != ControllerPerformanceClass::SerialPABotBase_Wired){
        throw UserSetupError(
            env.logger(),
            "This program requires a controller performance class of \"Wired\" for the Switch 2."
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

        increment_day_jp(context);

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



void DaySkipperJPN::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
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
