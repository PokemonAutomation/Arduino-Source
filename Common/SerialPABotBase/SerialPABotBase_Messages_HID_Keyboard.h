/*  SerialPABotBase Messages (HID Keyboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_HID_Keyboard_H
#define PokemonAutomation_SerialPABotBase_Messages_HID_Keyboard_H

#include "../ControllerStates/StandardHid_Keyboard_State.h"
#include "SerialPABotBase_Protocol.h"

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#elif __GNUC__
#define PABB_PACK   __attribute__((packed))
#else
#define PABB_PACK
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define PABB_MSG_COMMAND_HID_KEYBOARD_STATE     0x82
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_HID_Keyboard_State report;
} PABB_PACK pabb_Message_Command_HID_Keyboard_State;



#ifdef __cplusplus
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
