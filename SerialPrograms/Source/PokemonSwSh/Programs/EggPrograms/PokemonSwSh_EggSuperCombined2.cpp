/*  Egg Super-Combined 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_EggHelpers.h"
#include "PokemonSwSh_EggCombinedShared.h"
#include "PokemonSwSh_EggSuperCombined2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

EggSuperCombined2::EggSuperCombined2()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_31KB,
        "Egg Super-Combined 2",
        "NativePrograms/EggSuperCombined2.md",
        "Fetch and hatch eggs at the same time. (Fastest - 1700 eggs/day for 5120-step)"
    )
    , BOXES_TO_RELEASE(
        "<b>Boxes to Release:</b><br>Start by releasing this many boxes.",
        2, 0, 32
    )
    , BOXES_TO_SKIP(
        "<b>Boxes to Skip:</b><br>Then skip this many boxes.",
        1, 0, 32
    )
    , BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        31, 0, 32
    )
    , FETCHES_PER_BATCH(
        "<b>Fetches per Batch:</b><br>For each batch of eggs, attempt this many egg fetches.",
        6.0, 0, 7
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAFETY_TIME(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        "8 * TICKS_PER_SECOND"
    )
    , EARLY_HATCH_SAFETY(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        "5 * TICKS_PER_SECOND"
    )
    , HATCH_DELAY(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        "88 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&BOXES_TO_RELEASE, "BOXES_TO_RELEASE");
    m_options.emplace_back(&BOXES_TO_SKIP, "BOXES_TO_SKIP");
    m_options.emplace_back(&BOXES_TO_HATCH, "BOXES_TO_HATCH");
    m_options.emplace_back(&STEPS_TO_HATCH, "STEPS_TO_HATCH");
    m_options.emplace_back(&FETCHES_PER_BATCH, "FETCHES_PER_BATCH");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&SAFETY_TIME, "SAFETY_TIME");
    m_options.emplace_back(&EARLY_HATCH_SAFETY, "EARLY_HATCH_SAFETY");
    m_options.emplace_back(&HATCH_DELAY, "HATCH_DELAY");
}

void EggSuperCombined2::program(SingleSwitchProgramEnvironment& env) const{
    EggCombinedSession session{
        .BOXES_TO_HATCH = BOXES_TO_HATCH,
        .STEPS_TO_HATCH = STEPS_TO_HATCH,
        .FETCHES_PER_BATCH = (float)FETCHES_PER_BATCH,
        .SAFETY_TIME = SAFETY_TIME,
        .EARLY_HATCH_SAFETY = EARLY_HATCH_SAFETY,
        .HATCH_DELAY = HATCH_DELAY,
        .TOUCH_DATE_INTERVAL = TOUCH_DATE_INTERVAL,
    };

    grip_menu_connect_go_home(env.console);
    resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);

    //  Mass Release
    ssf_press_button2(env.console, BUTTON_X, OVERWORLD_TO_MENU_DELAY, 20);
    ssf_press_button1(env.console, BUTTON_A, 200);
    ssf_press_button1(env.console, BUTTON_R, 250);
    release_boxes(env.console, BOXES_TO_RELEASE, BOX_SCROLL_DELAY, BOX_CHANGE_DELAY);

    //  Skip Boxes
    for (uint8_t c = 0; c <= BOXES_TO_SKIP; c++){
        ssf_press_button1(env.console, BUTTON_R, 60);
    }
    pbf_mash_button(env.console, BUTTON_B, 600);

    session.eggcombined2_body(env);

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

