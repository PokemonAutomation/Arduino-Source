/*  Move Box Arrow
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA_MoveBoxArrow.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


MoveBoxArrow_Descriptor::MoveBoxArrow_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:MoveBoxArrow",
        STRING_POKEMON + " LZA", "Move Box Arrow",
        "",
        "Test program to move the box cursor to a specified location. "
        "Used for debugging box cursor movement. "
        "Start this program while in the box system view.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}


MoveBoxArrow::MoveBoxArrow()
    : TARGET_ROW(
        "<b>Target Row:</b><br>Row to move cursor to (0 = party row, 1-5 = box rows).",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 5
    )
    , TARGET_COL(
        "<b>Target Column:</b><br>Column to move cursor to (0-5).",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 5
    )
{
    PA_ADD_OPTION(TARGET_ROW);
    PA_ADD_OPTION(TARGET_COL);
}


void MoveBoxArrow::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    env.log("Starting Move Box Arrow test program...");
    env.log("Target position: Row " + std::to_string(TARGET_ROW) + ", Col " + std::to_string(TARGET_COL));

    // Create box detector
    BoxDetector detector(COLOR_RED, &env.console.overlay());

    // Detect current position
    VideoSnapshot screen = env.console.video().snapshot();
    detector.detect(screen);
    BoxCursorCoordinates current = detector.detected_location();

    if (current.row == BoxCursorCoordinates::INVALID || current.col == BoxCursorCoordinates::INVALID){
        env.log("Failed to detect box cursor. Make sure you are in the box system view!", COLOR_RED);
        return;
    }

    env.log("Current position: Row " + std::to_string(current.row) + ", Col " + std::to_string(current.col), COLOR_BLUE);

    // Move cursor to target position
    env.log("Moving cursor to target position...", COLOR_PURPLE);
    detector.move_cursor(env.program_info(), env.console, context, TARGET_ROW, TARGET_COL);

    env.log("Cursor movement complete!", COLOR_GREEN);
}




}
}
}
