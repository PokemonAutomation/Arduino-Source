/*  Nintendo Switch - OEM Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <string.h>
#include "NintendoSwitch_OemController_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


const OemController_RumbleState OemController_RumbleState_NEUTRAL_STATE = {
    .left = {0x00, 0x01, 0x40, 0x40},
    .right = {0x00, 0x01, 0x40, 0x40},
};



extern const OemController_State0x3f OemController_State0x3f_NEUTRAL_STATE = {
    .button0 = 0,
    .button1 = 0,
    .dpad = 8,
    .left_joystick = {0x00, 0x80, 0x00, 0x80},
    .right_joystick = {0x00, 0x80, 0x00, 0x80},
};

const OemController_State0x30 OemController_State0x30_NEUTRAL_STATE = {
    .buttons = {
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
    },
    .gyro = {},
};

bool OemController_State0x30_equals(
    const OemController_State0x30* state0,
    const OemController_State0x30* state1
){
    return memcmp(state0, state1, sizeof(OemController_State0x30)) == 0;
}


}
}
