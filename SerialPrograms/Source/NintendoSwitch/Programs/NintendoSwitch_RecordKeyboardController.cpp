/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

// #include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
// #include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
// #include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_RecordKeyboardController.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


RecordKeyboardController_Descriptor::RecordKeyboardController_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:RecordKeyboardController",
        "Nintendo Switch", "Record Keyboard Controller",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/RecordKeyboardController.md",
        "Record actions from the keyboard controller, then play it back.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



RecordKeyboardController::~RecordKeyboardController(){
    
    // m_keyboard_manager->remove_listener(*this);
}

RecordKeyboardController::RecordKeyboardController(){
}



void RecordKeyboardController::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    
    
}

void RecordKeyboardController::monitor_keyboard_events(ProControllerContext& context){
    // ProController::KeyboardManager& keyboard_manager = *context.controller().m_keyboard_manager;
    // keyboard_manager.add_listener(*this);


    // context.controller().m_keyboard_manager ->m_keyboard_manager->add_listener(*this);  
    // keyboard_manager  
}

    // m_keyboard_manager->remove_listener(*this);


void RecordKeyboardController::on_keyboard_command_sent(WallClock time_stamp, const ControllerState& state){
    cout << "keyboard_command_sent" << endl;
}
void RecordKeyboardController::on_keyboard_command_stopped(WallClock time_stamp){
    cout << "keyboard_command_stopped" << endl;
}



}
}

