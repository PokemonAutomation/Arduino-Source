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
} pa_NintendoSwitch_WirelessController_State0x30_Buttons;

typedef struct{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t rotation_x;
    int16_t rotation_y;
    int16_t rotation_z;
} pa_NintendoSwitch_WirelessController_State0x30_Gyro;

typedef struct{
    pa_NintendoSwitch_WirelessController_State0x30_Gyro time0;
    pa_NintendoSwitch_WirelessController_State0x30_Gyro time1;
    pa_NintendoSwitch_WirelessController_State0x30_Gyro time2;
} pa_NintendoSwitch_WirelessController_State0x30_GyroX3;

typedef struct{
    pa_NintendoSwitch_WirelessController_State0x30_Buttons buttons;
    pa_NintendoSwitch_WirelessController_State0x30_GyroX3 gyro;
} pa_NintendoSwitch_WirelessController_State0x30;




extern const pa_NintendoSwitch_WirelessController_State0x30 pa_NintendoSwitch_WirelessController_State0x30_NEUTRAL_STATE;

bool pa_NintendoSwitch_WirelessController_State0x30_equals(
    const pa_NintendoSwitch_WirelessController_State0x30* state0,
    const pa_NintendoSwitch_WirelessController_State0x30* state1
);



#ifdef __cplusplus
}
#endif
#endif
