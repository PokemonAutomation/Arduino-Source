/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_Protocol_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Protocol_PushButtons_H

//#ifdef __AVR__
//#include "NativePrograms/NintendoSwitch/Framework/Master.h"
//#endif
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

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


#define PABB_MSG_COMMAND_PBF_WAIT                   0x90
typedef struct{
    seqnum_t seqnum;
    uint16_t ticks;
} PABB_PACK pabb_pbf_wait;

#define PABB_MSG_COMMAND_PBF_PRESS_BUTTON           0x91
typedef struct{
    seqnum_t seqnum;
    Button button;
    uint16_t hold_ticks;
    uint16_t release_ticks;
} PABB_PACK pabb_pbf_press_button;

#define PABB_MSG_COMMAND_PBF_PRESS_DPAD             0x92
typedef struct{
    seqnum_t seqnum;
    DpadPosition dpad;
    uint16_t hold_ticks;
    uint16_t release_ticks;
} PABB_PACK pabb_pbf_press_dpad;

#define PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L        0x93
#define PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R        0x94
typedef struct{
    seqnum_t seqnum;
    uint8_t x;
    uint8_t y;
    uint16_t hold_ticks;
    uint16_t release_ticks;
} PABB_PACK pabb_pbf_move_joystick;

#define PABB_MSG_COMMAND_MASH_BUTTON                0x95
typedef struct{
    seqnum_t seqnum;
    Button button;
    uint16_t ticks;
} PABB_PACK pabb_pbf_mash_button;

#define PABB_MSG_CONTROLLER_STATE                   0x9f
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


#ifdef __cplusplus
}
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
