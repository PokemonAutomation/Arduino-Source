/*  Pokemon Automation Superscalar Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_Protocol_Superscalar_H
#define PokemonAutomation_NintendoSwitch_Protocol_Superscalar_H

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


#define PABB_MSG_COMMAND_DEBUG_I32              0xf0
typedef struct{
    seqnum_t seqnum;
    uint32_t data;
} PABB_PACK pabb_command_debug;

#define PABB_MSG_COMMAND_SSF_FLUSH_PIPELINE     0xf1
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_ssf_flush_pipeline;

#define PABB_MSG_COMMAND_SSF_DO_NOTHING         0xf2
typedef struct{
    seqnum_t seqnum;
    uint16_t ticks;
} PABB_PACK pabb_ssf_do_nothing;


#define PABB_MSG_COMMAND_SSF_PRESS_BUTTON       0xf3
typedef struct{
    seqnum_t seqnum;
    Button button;
    uint16_t delay;
    uint16_t hold;
    uint8_t cool;
} PABB_PACK pabb_ssf_press_button;


#define PABB_MSG_COMMAND_SSF_PRESS_DPAD         0xf4
typedef struct{
    seqnum_t seqnum;
    DpadPosition position;
    uint16_t delay;
    uint16_t hold;
    uint8_t cool;
} PABB_PACK pabb_ssf_press_dpad;


#define PABB_MSG_COMMAND_SSF_PRESS_JOYSTICK_L   0xf5
#define PABB_MSG_COMMAND_SSF_PRESS_JOYSTICK_R   0xf6
typedef struct{
    seqnum_t seqnum;
    uint8_t x;
    uint8_t y;
    uint16_t delay;
    uint16_t hold;
    uint8_t cool;
} PABB_PACK pabb_ssf_press_joystick;

#define PABB_MSG_COMMAND_SSF_MASH1_BUTTON       0xf7
typedef struct{
    seqnum_t seqnum;
    Button button;
    uint16_t ticks;
} PABB_PACK pabb_ssf_mash1_button;

#define PABB_MSG_COMMAND_SSF_MASH2_BUTTON       0xf8
typedef struct{
    seqnum_t seqnum;
    Button button0;
    Button button1;
    uint16_t ticks;
} PABB_PACK pabb_ssf_mash2_button;

#define PABB_MSG_COMMAND_SSF_MASH_AZS           0xf9
typedef struct{
    seqnum_t seqnum;
    uint16_t ticks;
} PABB_PACK pabb_ssf_mash_AZs;

#define PABB_MSG_COMMAND_SSF_SCROLL             0xfa
#define ssf_ScrollDirection     uint16_t
typedef struct{
    seqnum_t seqnum;
    ssf_ScrollDirection direction;
    uint16_t delay;
    uint16_t hold;
    uint8_t cool;
} PABB_PACK pabb_ssf_issue_scroll;


#ifdef __cplusplus
}
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
