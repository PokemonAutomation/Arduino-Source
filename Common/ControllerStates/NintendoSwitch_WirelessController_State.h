/*  Nintendo Switch - Wireless Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_WirelessController_State_H
#define PokemonAutomation_NintendoSwitch_WirelessController_State_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif




typedef struct{
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t left_joystick[3];
    uint8_t right_joystick[3];
    uint8_t vibrator;
} pabb_NintendoSwitch_WirelessController_State0x30_Buttons;

typedef struct{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t rotation_x;
    int16_t rotation_y;
    int16_t rotation_z;
} pabb_NintendoSwitch_WirelessController_State0x30_Gyro;

typedef struct{
    pabb_NintendoSwitch_WirelessController_State0x30_Gyro time0;
    pabb_NintendoSwitch_WirelessController_State0x30_Gyro time1;
    pabb_NintendoSwitch_WirelessController_State0x30_Gyro time2;
} pabb_NintendoSwitch_WirelessController_State0x30_GyroX3;

typedef struct{
    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons;
    pabb_NintendoSwitch_WirelessController_State0x30_GyroX3 gyro;
} pabb_NintendoSwitch_WirelessController_State0x30;




extern const pabb_NintendoSwitch_WirelessController_State0x30 pabb_NintendoSwitch_WirelessController_State0x30_NEUTRAL_STATE;

bool pabb_NintendoSwitch_WirelessController_State0x30_equals(
    const pabb_NintendoSwitch_WirelessController_State0x30* state0,
    const pabb_NintendoSwitch_WirelessController_State0x30* state1
);



#ifdef __cplusplus
}
#endif
#endif
