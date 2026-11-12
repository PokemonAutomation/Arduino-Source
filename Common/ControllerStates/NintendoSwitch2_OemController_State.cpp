/*  Nintendo Switch 2 - OEM Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <string.h>
#include "NintendoSwitch2_OemController_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch2{


//  The wire payload is 63 bytes; the structs hold the CC-filled portion
//  (the firmware appends the counter / power-info prefix and the USB
//  report ID byte): 59 bytes for 0x05, 61 for the native reports.
static_assert(sizeof(OemController_State0x05) == 59, "Report 0x05 state must be 59 bytes");
static_assert(sizeof(OemController_State0x07) == 61, "Report 0x07 state must be 61 bytes");
static_assert(sizeof(OemController_State0x08) == 61, "Report 0x08 state must be 61 bytes");
static_assert(sizeof(OemController_State0x09) == 61, "Report 0x09 state must be 61 bytes");
static_assert(sizeof(OemController_State0x0A) == 61, "Report 0x0A state must be 61 bytes");


const OemController_RumbleState OemController_RumbleState_NEUTRAL_STATE = {
    //  Zero amplitude is silent.
    .left = {},
    .right = {},
};


const OemController_RumbleStateGC OemController_RumbleStateGC_NEUTRAL_STATE = {
    .motor = {},
};


const OemController_State0x05 OemController_State0x05_NEUTRAL_STATE = {
    .buttons = {},
    .unknown_0x08 = {},
    .left_stick = {0x00, 0x08, 0x80},
    .right_stick = {0x00, 0x08, 0x80},
    .optical = {},
    .unknown_0x18 = 0,
    .magnetometer = {},
    .battery_voltage_mv = 0,
    .charging_state = 0,
    .battery_current = 0,
    .unknown_0x24 = {},
    .unknown_0x29 = 0,
    .motion = {},
    .left_trigger = 0,
    .right_trigger = 0,
    .reserved = 0,
};


bool OemController_State0x05_equals(
    const OemController_State0x05* state0,
    const OemController_State0x05* state1
){
    return memcmp(state0, state1, sizeof(OemController_State0x05)) == 0;
}


const OemController_State0x07 OemController_State0x07_NEUTRAL_STATE = {
    .buttons0 = 0,
    .buttons1 = 0,
    .unknown_0x04 = 0x07,
    .stick = {0x00, 0x08, 0x80},
    .unknown_0x08 = 0,
    .optical = {},
    .motion_length = 0,
    .motion_data = {},
    .reserved = {},
};


bool OemController_State0x07_equals(
    const OemController_State0x07* state0,
    const OemController_State0x07* state1
){
    return memcmp(state0, state1, sizeof(OemController_State0x07)) == 0;
}


const OemController_State0x08 OemController_State0x08_NEUTRAL_STATE = {
    .buttons0 = 0,
    .buttons1 = 0,
    .unknown_0x04 = 0x07,
    .stick = {0x00, 0x08, 0x80},
    .unknown_0x08 = 0,
    .optical = {},
    .nfc_state = 0,
    .motion_length = 0,
    .motion_data = {},
    .reserved = {},
};


bool OemController_State0x08_equals(
    const OemController_State0x08* state0,
    const OemController_State0x08* state1
){
    return memcmp(state0, state1, sizeof(OemController_State0x08)) == 0;
}


const OemController_State0x09 OemController_State0x09_NEUTRAL_STATE = {
    .buttons = {},
    .left_stick = {0x00, 0x08, 0x80},
    .right_stick = {0x00, 0x08, 0x80},
    .unknown_0x0B = 0x30,
    .nfc_state = 0,
    .headset_audio_state = 0,
    .motion_length = 0,
    .motion_data = {},
    .reserved = {},
};


bool OemController_State0x09_equals(
    const OemController_State0x09* state0,
    const OemController_State0x09* state1
){
    return memcmp(state0, state1, sizeof(OemController_State0x09)) == 0;
}


const OemController_State0x0A OemController_State0x0A_NEUTRAL_STATE = {
    .buttons = {},
    .left_stick = {0x00, 0x08, 0x80},
    .right_stick = {0x00, 0x08, 0x80},
    .unknown_0x0B = 0x30,
    .left_trigger = 0,
    .right_trigger = 0,
    .motion_length = 0,
    .motion_data = {},
    .reserved = {},
};


bool OemController_State0x0A_equals(
    const OemController_State0x0A* state0,
    const OemController_State0x0A* state1
){
    return memcmp(state0, state1, sizeof(OemController_State0x0A)) == 0;
}


}
}
