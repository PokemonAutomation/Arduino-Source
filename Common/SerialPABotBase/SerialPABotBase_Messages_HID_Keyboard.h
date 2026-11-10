/*  SerialPABotBase Messages (HID Keyboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_HID_Keyboard_H
#define PokemonAutomation_SerialPABotBase_Messages_HID_Keyboard_H

#include "../ControllerStates/StandardHid_Keyboard_State.h"
#include "SerialPABotBase_Protocol.h"

#pragma pack(push, 1)


#ifdef __cplusplus
extern "C" {
#endif


#define PABB_MSG_COMMAND_HID_KEYBOARD_STATE     0x82
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_HID_Keyboard_State report;
} pabb_Message_Command_HID_Keyboard_State;



#ifdef __cplusplus
}
#endif


#pragma pack(pop)

#endif
