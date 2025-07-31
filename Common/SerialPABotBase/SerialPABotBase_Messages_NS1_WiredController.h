/*  SerialPABotBase Messages (Nintendo Switch 1 Wired Controller)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_NS1_WiredController_H
#define PokemonAutomation_SerialPABotBase_Messages_NS1_WiredController_H

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
namespace PokemonAutomation{
namespace SerialPABotBase{
#endif


#define PABB_MSG_REPORT_NS1_WIRED_CONTROLLER    0x90
typedef struct{
    uint8_t buttons0;
    uint8_t buttons1;
    uint8_t dpad;
    uint8_t left_joystick_x;
    uint8_t left_joystick_y;
    uint8_t right_joystick_x;
    uint8_t right_joystick_y;
} pabb_Report_NS1_WiredController;
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_Report_NS1_WiredController report;
} PABB_PACK pabb_Message_Report_NS1_WiredController;



#ifdef __cplusplus
}
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
