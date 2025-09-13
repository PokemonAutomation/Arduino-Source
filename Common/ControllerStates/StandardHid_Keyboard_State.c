/*  Standard HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "HID_Keyboard_State.h"


const pabb_HID_Keyboard_State pabb_HID_Keyboard_State_NEUTRAL_STATE = {};

bool pabb_HID_Keyboard_State_equals(
    const pabb_HID_Keyboard_State* state0,
    const pabb_HID_Keyboard_State* state1
){
    return memcmp(state0, state1, sizeof(pabb_HID_Keyboard_State)) == 0;
}
