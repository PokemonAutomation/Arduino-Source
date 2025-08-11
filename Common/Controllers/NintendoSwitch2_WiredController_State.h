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
} pa_NintendoSwitch2_WiredController_State;

extern const pa_NintendoSwitch2_WiredController_State pa_NintendoSwitch2_WiredController_State_NEUTRAL_STATE;

bool pa_NintendoSwitch2_WiredController_State_equals(
    const pa_NintendoSwitch2_WiredController_State* state0,
    const pa_NintendoSwitch2_WiredController_State* state1
);



#ifdef __cplusplus
}
#endif
#endif
