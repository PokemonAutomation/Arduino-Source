/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Options/TestPathMakerTable.h"
#include "TestPathMaker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


TestPathMaker_Descriptor::TestPathMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "Path Maker Test",
        "Test", "Path Maker Test",
        "",
        "Create paths to test routes",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

TestPathMaker::TestPathMaker(){
    PA_ADD_OPTION(PATH);
}

void TestPathMaker::run_path(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    for (size_t action_index = 0; action_index < PATH.num_actions(); action_index++){
        execute_action(env.console, context, action_index);
        context.wait_for_all_requests();
    }
}

void TestPathMaker::execute_action(ConsoleHandle& console, BotBaseContext& context, size_t action_index){
    const auto& row = PATH.get_action(action_index);
    console.log("Execute action " + PathAction_NAMES[(size_t)row.action]);
    switch(row.action){
    case PathAction::LEFT_JOYSTICK:
        pbf_move_left_joystick(context, row.x_axis, row.y_axis, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::RIGHT_JOYSTICK:
        pbf_move_right_joystick(context, row.x_axis, row.y_axis, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::B:
        pbf_press_button(context, BUTTON_B, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::A:
        pbf_press_button(context, BUTTON_A, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::Y:
        pbf_press_button(context, BUTTON_Y, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::X:
        pbf_press_button(context, BUTTON_X, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::R:
        pbf_press_button(context, BUTTON_R, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::L:
        pbf_press_button(context, BUTTON_L, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::ZR:
        pbf_press_button(context, BUTTON_ZR,row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::ZL:
        pbf_press_button(context, BUTTON_ZL, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::PLUS:
        pbf_press_button(context, BUTTON_PLUS, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::MINUS:
        pbf_press_button(context, BUTTON_MINUS, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::DPADLEFT:
        pbf_press_dpad(context, DPAD_LEFT, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::DPADRIGHT:
        pbf_press_dpad(context, DPAD_RIGHT, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::DPADUP:
        pbf_press_dpad(context, DPAD_UP, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::DPADDOWN:
        pbf_press_dpad(context, DPAD_DOWN, row.button_hold_ticks, row.button_release_ticks);
        break;
    case PathAction::WAIT:
        pbf_wait(context, (row.wait_ticks * TICKS_PER_SECOND));
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
