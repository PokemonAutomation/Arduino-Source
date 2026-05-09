/*  Standard HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StandardHid_Keyboard_State_H
#define PokemonAutomation_StandardHid_Keyboard_State_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


//
//  Console <-> Controller
//

typedef struct{
    uint8_t data;
} pabb_HID_Keyboard_LockLEDs;


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


//
//  Controller <-> CC
//

typedef struct{
    //  Bit 0: Connected
    //  Bit 4: Num Lock
    //  Bit 5: Caps Lock
    //  Bit 6: Scroll Lock
    uint8_t status;
} pabb_HID_Keyboard_Status;


#ifdef __cplusplus
}
#endif
#endif
