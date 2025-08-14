/*  Nintendo Switch 2 - Wired Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_WiredController_State_H
#define PokemonAutomation_NintendoSwitch2_WiredController_State_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct{
    uint8_t buttons0;
    uint8_t buttons1;
    uint8_t dpad_byte;
    uint8_t left_joystick_x;
    uint8_t left_joystick_y;
    uint8_t right_joystick_x;
    uint8_t right_joystick_y;
} pabb_NintendoSwitch2_WiredController_State;

extern const pabb_NintendoSwitch2_WiredController_State pabb_NintendoSwitch2_WiredController_State_NEUTRAL_STATE;

bool pabb_NintendoSwitch2_WiredController_State_equals(
    const pabb_NintendoSwitch2_WiredController_State* state0,
    const pabb_NintendoSwitch2_WiredController_State* state1
);



#ifdef __cplusplus
}
#endif
#endif
