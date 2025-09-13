/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RecordKeyboardController_H
#define PokemonAutomation_NintendoSwitch_RecordKeyboardController_H

#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class ProControllerState;

class RecordKeyboardController_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RecordKeyboardController_Descriptor();
};



class RecordKeyboardController : public SingleSwitchProgramInstance, public KeyboardEventHandler::KeyboardListener{ 
public:
    ~RecordKeyboardController();
    RecordKeyboardController();
    

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    virtual void on_keyboard_command_sent(WallClock time_stamp, const ControllerState& state) override;
    virtual void on_keyboard_command_stopped(WallClock time_stamp) override;

};




}
}
#endif



