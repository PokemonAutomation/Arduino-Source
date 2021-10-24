/*  Misc. Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PokemonSwSh_Protocol_Misc_H
#define PokemonAutomation_PokemonSwSh_Protocol_Misc_H

#ifdef __AVR__
#include "NativePrograms/NintendoSwitch/Framework/Master.h"
#endif
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Protocols
#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif
////////////////////////////////////////////////////////////////////////////////

#define PABB_MSG_CONTROLLER_STATE                               0x9f
typedef struct{
    seqnum_t seqnum;
    Button button;
    DpadPosition dpad;
    uint8_t left_joystick_x;
    uint8_t left_joystick_y;
    uint8_t right_joystick_x;
    uint8_t right_joystick_y;
    uint8_t ticks;
} PABB_PACK pabb_controller_state;

#define PABB_MSG_COMMAND_MASH_A                                 0xa0
typedef struct{
    seqnum_t seqnum;
    uint16_t ticks;
} PABB_PACK pabb_mashA;

#define PABB_MSG_COMMAND_IOA_BACKOUT                            0xa2
typedef struct{
    seqnum_t seqnum;
    uint16_t pokemon_to_menu_delay;
} PABB_PACK pabb_IoA_backout;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

