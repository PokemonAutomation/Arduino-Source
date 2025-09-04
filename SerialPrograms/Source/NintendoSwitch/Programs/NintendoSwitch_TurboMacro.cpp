/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Options/TurboMacroTable.h"
#include "NintendoSwitch_TurboMacro.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


TurboMacro_Descriptor::TurboMacro_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:TurboMacro",
        "Nintendo Switch", "Turbo Macro",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/TurboMacro.md",
        "Create macros",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

TurboMacro::TurboMacro()
    : LOOP(
        "<b>Number of times to loop:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0
        )
{
    PA_ADD_OPTION(LOOP);
    PA_ADD_OPTION(MACRO);
}

void TurboMacro::run_macro(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::vector<std::unique_ptr<TurboMacroRow>> table = MACRO.copy_snapshot();
    for (const std::unique_ptr<TurboMacroRow>& row : table){
        execute_action(env.console, context, *row);
    }
}

void TurboMacro::execute_action(
    VideoStream& stream, ProControllerContext& context,
    const TurboMacroRow& row
){
    stream.log("Execute action " + row.action.current_display());
    const TurboMacroCell& cell = row.parameters;
    switch(row.action){
    case TurboMacroAction::LEFT_JOYSTICK:
        pbf_move_left_joystick(context, cell.x_axis, cell.y_axis, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::RIGHT_JOYSTICK:
        pbf_move_right_joystick(context, cell.x_axis, cell.y_axis, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::LEFT_JOY_CLICK:
        pbf_press_button(context, BUTTON_LCLICK, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::RIGHT_JOY_CLICK:
        pbf_press_button(context, BUTTON_RCLICK, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::B:
        pbf_press_button(context, BUTTON_B, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::A:
        pbf_press_button(context, BUTTON_A, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::Y:
        pbf_press_button(context, BUTTON_Y, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::X:
        pbf_press_button(context, BUTTON_X, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::R:
        pbf_press_button(context, BUTTON_R, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::L:
        pbf_press_button(context, BUTTON_L, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::ZR:
        pbf_press_button(context, BUTTON_ZR,cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::ZL:
        pbf_press_button(context, BUTTON_ZL, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::PLUS:
        pbf_press_button(context, BUTTON_PLUS, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::MINUS:
        pbf_press_button(context, BUTTON_MINUS, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::DPADLEFT:
        pbf_press_dpad(context, DPAD_LEFT, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::DPADRIGHT:
        pbf_press_dpad(context, DPAD_RIGHT, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::DPADUP:
        pbf_press_dpad(context, DPAD_UP, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::DPADDOWN:
        pbf_press_dpad(context, DPAD_DOWN, cell.button_hold, cell.button_release);
        break;
    case TurboMacroAction::WAIT:
        pbf_wait(context, cell.wait);
    default:
        break;
    }
}

void TurboMacro::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    //  Connect the controller.
    //pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    for (uint32_t c = 0; c < LOOP; c++){
        run_macro(env, context);
    }
}


}
}
