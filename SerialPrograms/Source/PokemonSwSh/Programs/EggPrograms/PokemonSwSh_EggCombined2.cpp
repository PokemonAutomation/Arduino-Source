/*  Egg Combined 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_EggHelpers.h"
#include "PokemonSwSh_EggCombinedShared.h"
#include "PokemonSwSh_EggCombined2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


EggCombined2_Descriptor::EggCombined2_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:EggCombined2",
        "Egg Combined 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggCombined2.md",
        "Fetch and hatch eggs at the same time. (Fastest - 1700 eggs/day for 5120-step)",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



EggCombined2::EggCombined2(const EggCombined2_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        32, 0, 32
    )
    , FETCHES_PER_BATCH(
        "<b>Fetches per Batch:</b><br>For each batch of eggs, attempt this many egg fetches.",
        6.0, 0, 7
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAFETY_TIME(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        "8 * TICKS_PER_SECOND"
    )
    , EARLY_HATCH_SAFETY(
        "<b>Early Hatch Safety:</b><br>Eggs will not hatch early by more than this period.",
        "5 * TICKS_PER_SECOND"
    )
    , HATCH_DELAY(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        "88 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_OPTION(FETCHES_PER_BATCH);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME);
    PA_ADD_OPTION(EARLY_HATCH_SAFETY);
    PA_ADD_OPTION(HATCH_DELAY);
}

void EggCombined2::program(SingleSwitchProgramEnvironment& env){
    EggCombinedSession session{
        .BOXES_TO_HATCH = BOXES_TO_HATCH,
        .STEPS_TO_HATCH = STEPS_TO_HATCH,
        .FETCHES_PER_BATCH = (float)FETCHES_PER_BATCH,
        .SAFETY_TIME = SAFETY_TIME,
        .EARLY_HATCH_SAFETY = EARLY_HATCH_SAFETY,
        .HATCH_DELAY = HATCH_DELAY,
        .TOUCH_DATE_INTERVAL = TOUCH_DATE_INTERVAL,
    };

    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    session.eggcombined2_body(env);

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

