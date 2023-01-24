/*  Test Path Maker Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Options/TestPathMakerTable.h"
#include "PathMaker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


TestPathMaker_Descriptor::TestPathMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:PathMaker",
        "Nintendo Switch", "Path Maker",
        "",
        "Create paths to test routes",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

TestPathMaker::TestPathMaker(){
    PA_ADD_OPTION(PATH);
}

void TestPathMaker::run_path(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    std::vector<std::unique_ptr<PathMakerRow2>> table = PATH.copy_snapshot();
    for (const std::unique_ptr<PathMakerRow2>& row : table){
        execute_action(env.console, context, *row);
        context.wait_for_all_requests();
    }
}

void TestPathMaker::execute_action(ConsoleHandle& console, BotBaseContext& context, const PathMakerRow2& row){
    console.log("Execute action " + row.action.current_display());
    const PathMakerCell& cell = row.parameters;
    switch(row.action){
    case PathAction::LEFT_JOYSTICK:
        pbf_move_left_joystick(context, cell.x_axis, cell.y_axis, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::RIGHT_JOYSTICK:
        pbf_move_right_joystick(context, cell.x_axis, cell.y_axis, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::B:
        pbf_press_button(context, BUTTON_B, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::A:
        pbf_press_button(context, BUTTON_A, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::Y:
        pbf_press_button(context, BUTTON_Y, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::X:
        pbf_press_button(context, BUTTON_X, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::R:
        pbf_press_button(context, BUTTON_R, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::L:
        pbf_press_button(context, BUTTON_L, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::ZR:
        pbf_press_button(context, BUTTON_ZR,cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::ZL:
        pbf_press_button(context, BUTTON_ZL, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::PLUS:
        pbf_press_button(context, BUTTON_PLUS, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::MINUS:
        pbf_press_button(context, BUTTON_MINUS, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::DPADLEFT:
        pbf_press_dpad(context, DPAD_LEFT, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::DPADRIGHT:
        pbf_press_dpad(context, DPAD_RIGHT, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::DPADUP:
        pbf_press_dpad(context, DPAD_UP, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::DPADDOWN:
        pbf_press_dpad(context, DPAD_DOWN, cell.button_hold_ticks, cell.button_release_ticks);
        break;
    case PathAction::WAIT:
        pbf_wait(context, (cell.wait_ticks * TICKS_PER_SECOND));
    default:
        break;
    }
    context.wait_for_all_requests();
}

void TestPathMaker::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    run_path(env, context);
}


}
}
