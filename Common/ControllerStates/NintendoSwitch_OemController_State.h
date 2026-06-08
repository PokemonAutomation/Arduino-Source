/*  Nintendo Switch - OEM Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_OemController_State_H
#define PokemonAutomation_NintendoSwitch_OemController_State_H

#include <stdbool.h>
#include <stdint.h>

namespace PokemonAutomation{
namespace NintendoSwitch{



//
//  Console <-> Controller
//


struct OemController_RumbleState{
    uint8_t left[4];
    uint8_t right[4];
};

extern const OemController_RumbleState OemController_RumbleState_NEUTRAL_STATE;


//
//  Report 0x3f (no longer used)
//

struct OemController_State0x3f{
    uint8_t button0;
    uint8_t button1;
    uint8_t dpad;
    uint8_t left_joystick[4];
    uint8_t right_joystick[4];
};

extern const OemController_State0x3f OemController_State0x3f_NEUTRAL_STATE;


//
//  Report 0x30
//

struct OemController_State0x30_Buttons{
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t left_joystick[3];
    uint8_t right_joystick[3];
    uint8_t vibrator;
};

struct OemController_State0x30_Gyro{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t rotation_x;
    int16_t rotation_y;
    int16_t rotation_z;
};

struct OemController_State0x30_GyroX3{
    OemController_State0x30_Gyro time0;
    OemController_State0x30_Gyro time1;
    OemController_State0x30_Gyro time2;
};

struct OemController_State0x30{
    OemController_State0x30_Buttons buttons;
    OemController_State0x30_GyroX3 gyro;
};

extern const OemController_State0x30 OemController_State0x30_NEUTRAL_STATE;

bool OemController_State0x30_equals(
    const OemController_State0x30* state0,
    const OemController_State0x30* state1
);


//
//  Controller <-> CC
//

struct OemController_Status{
    //  Bit 0: Connected
    //  Bit 1: Ready
    //  Bit 2: Paired
    uint8_t status;

    uint8_t player_lights;

    uint8_t paired_mac_address[6];
};

enum class DisconnectReason{
    HOST_SHUTDOWN,
    CONNECTION_REJECTED,
};



}
}
#endif
