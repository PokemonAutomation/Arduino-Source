/*  Standard HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_HID_Keyboard_State_H
#define PokemonAutomation_HID_Keyboard_State_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct{
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t key[6];
} pabb_HID_Keyboard_State;

extern const pabb_HID_Keyboard_State pabb_HID_Keyboard_State_NEUTRAL_STATE;

bool pabb_HID_Keyboard_State_equals(
    const pabb_HID_Keyboard_State* state0,
    const pabb_HID_Keyboard_State* state1
);



#ifdef __cplusplus
}
#endif
#endif
