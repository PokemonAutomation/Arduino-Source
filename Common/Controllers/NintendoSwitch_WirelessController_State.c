/*  Nintendo Switch - Wireless Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <string.h>
#include "NintendoSwitch_WirelessController_State.h"


const pa_NintendoSwitch_WirelessController_State0x30 pa_NintendoSwitch_WirelessController_State0x30_NEUTRAL_STATE = {
    .buttons = {
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x80,
    },
    .gyro = {},
};

bool pa_NintendoSwitch_WirelessController_State0x30_equals(
    const pa_NintendoSwitch_WirelessController_State0x30* state0,
    const pa_NintendoSwitch_WirelessController_State0x30* state1
){
    return memcmp(state0, state1, sizeof(pa_NintendoSwitch_WirelessController_State0x30)) == 0;
}

