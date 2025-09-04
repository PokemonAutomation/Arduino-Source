/*  Egg Super-Combined 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_ReleaseHelpers.h"
#include "PokemonSwSh_EggCombinedShared.h"
#include "PokemonSwSh_EggSuperCombined2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


EggSuperCombined2_Descriptor::EggSuperCombined2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggSuperCombined2",
        STRING_POKEMON + " SwSh", "Egg Super-Combined 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggSuperCombined2.md",
        "Fetch and hatch eggs at the same time. (Fastest - 1700 eggs/day for 5120-step)",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
    )
{}



EggSuperCombined2::EggSuperCombined2()
    : BOXES_TO_RELEASE(
        "<b>Boxes to Release:</b><br>Start by releasing this many boxes.",
        LockMode::LOCK_WHILE_RUNNING,
        2, 0, 32
    )
    , BOXES_TO_SKIP(
        "<b>Boxes to Skip:</b><br>Then skip this many boxes.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 0, 32
    )
    , BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        31, 0, 32
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
        "<b>Early Hatch Time:</b><br>Eggs should not hatch more than this much early.",
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

    PA_ADD_OPTION(BOXES_TO_RELEASE);
    PA_ADD_OPTION(BOXES_TO_SKIP);
    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_OPTION(FETCHES_PER_BATCH);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME0);
    PA_ADD_OPTION(EARLY_HATCH_SAFETY0);
    PA_ADD_OPTION(HATCH_DELAY0);
}

void EggSuperCombined2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
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

    //  Mass Release
    ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
    ssf_press_button(context, BUTTON_A, 1600ms);
    ssf_press_button(context, BUTTON_R, 2000ms);
    release_boxes(context, BOXES_TO_RELEASE);

    //  Skip Boxes
    for (uint8_t c = 0; c <= BOXES_TO_SKIP; c++){
        ssf_press_button(context, BUTTON_R, 480ms);
    }
    pbf_mash_button(context, BUTTON_B, 600);

    session.eggcombined2_body(env.console, context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}

