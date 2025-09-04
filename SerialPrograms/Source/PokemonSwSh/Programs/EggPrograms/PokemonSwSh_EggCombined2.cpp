/*  Egg Combined 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_EggCombinedShared.h"
#include "PokemonSwSh_EggCombined2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


EggCombined2_Descriptor::EggCombined2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggCombined2",
        STRING_POKEMON + " SwSh", "Egg Combined 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggCombined2.md",
        "Fetch and hatch eggs at the same time. (Fastest - 1700 eggs/day for 5120-step)",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
    )
{}



EggCombined2::EggCombined2()
    : BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        32, 0, 32
    )
    , FETCHES_PER_BATCH(
        "<b>Fetches per Batch:</b><br>For each batch of eggs, attempt this many egg fetches.",
        LockMode::LOCK_WHILE_RUNNING,
        6.0, 0, 7
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAFETY_TIME0(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        LockMode::LOCK_WHILE_RUNNING,
        "12 s"
    )
    , EARLY_HATCH_SAFETY0(
        "<b>Early Hatch Safety:</b><br>Eggs will not hatch early by more than this period.",
        LockMode::LOCK_WHILE_RUNNING,
        "5 s"
    )
    , HATCH_DELAY0(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        LockMode::LOCK_WHILE_RUNNING,
        "88 s"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_OPTION(FETCHES_PER_BATCH);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME0);
    PA_ADD_OPTION(EARLY_HATCH_SAFETY0);
    PA_ADD_OPTION(HATCH_DELAY0);
}

void EggCombined2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    EggCombinedSession session{
        BOXES_TO_HATCH,
        STEPS_TO_HATCH,
        (float)FETCHES_PER_BATCH,
        SAFETY_TIME0,
        EARLY_HATCH_SAFETY0,
        HATCH_DELAY0,
        TOUCH_DATE_INTERVAL
    };

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    session.eggcombined2_body(env.console, context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}

