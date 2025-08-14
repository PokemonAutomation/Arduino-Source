/*  Nintendo Switch 2 - Wired Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "NintendoSwitch2_WiredController_State.h"


const pabb_NintendoSwitch2_WiredController_State pabb_NintendoSwitch2_WiredController_State_NEUTRAL_STATE = {
    .buttons0           =   0,
    .buttons1           =   0,
    .dpad_byte          =   8,
    .left_joystick_x    =   128,
    .left_joystick_y    =   128,
    .right_joystick_x   =   128,
    .right_joystick_y   =   128,
};

bool pabb_NintendoSwitch2_WiredController_State_equals(
    const pabb_NintendoSwitch2_WiredController_State* state0,
    const pabb_NintendoSwitch2_WiredController_State* state1
){
    return memcmp(state0, state1, sizeof(pabb_NintendoSwitch2_WiredController_State)) == 0;
}
