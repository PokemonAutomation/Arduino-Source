/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualControllerState_H
#define PokemonAutomation_NintendoSwitch_VirtualControllerState_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct ControllerState{
    Button buttons = 0;
    DpadPosition dpad = DPAD_NONE;
    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;

    bool operator==(const ControllerState& x) const;
    bool operator!=(const ControllerState& x) const;
};


struct ControllerDeltas{
    Button buttons = 0;
    int dpad_x = 0;
    int dpad_y = 0;
    int left_x = 0;
    int left_y = 0;
    int right_x = 0;
    int right_y = 0;

    void operator+=(const ControllerDeltas& x);

    //  Returns true if neutral.
    bool to_state(ControllerState& state) const;
};







}
}
#endif
