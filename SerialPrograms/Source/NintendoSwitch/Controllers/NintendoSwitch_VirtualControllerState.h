/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualControllerState_H
#define PokemonAutomation_NintendoSwitch_VirtualControllerState_H

//#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class ProControllerState;
class JoyconState;



struct ProControllerDeltas{
    Button buttons = BUTTON_NONE;
    int dpad_x = 0;
    int dpad_y = 0;
    int left_x = 0;
    int left_y = 0;
    int right_x = 0;
    int right_y = 0;

    void operator+=(const ProControllerDeltas& x);

    //  Returns true if neutral.
    bool to_state(ProControllerState& state) const;
};



struct JoyconDeltas{
    Button buttons = BUTTON_NONE;
    int joystick_x = 0;
    int joystick_y = 0;

    void operator+=(const JoyconDeltas& x);

    //  Returns true if neutral.
    bool to_state(JoyconState& state) const;
};






}
}
#endif
