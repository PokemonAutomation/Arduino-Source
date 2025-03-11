/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_Protocol_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Protocol_PushButtons_H

//#ifdef __AVR__
//#include "NativePrograms/NintendoSwitch/Framework/Master.h"
//#endif
#include "Common/Microcontroller/MessageProtocol.h"
//#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif


#ifdef __cplusplus
namespace PokemonAutomation{
namespace NintendoSwitch{
#endif



#define PABB_MSG_CONTROLLER_STATE                   0x9f
typedef struct{
    seqnum_t seqnum;
    uint16_t button;
    uint8_t dpad;
    uint8_t left_joystick_x;
    uint8_t left_joystick_y;
    uint8_t right_joystick_x;
    uint8_t right_joystick_y;
    uint8_t ticks;
} PABB_PACK pabb_controller_state;


#ifdef __cplusplus
}
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
