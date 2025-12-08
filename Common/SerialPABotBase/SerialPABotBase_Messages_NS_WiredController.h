/*  SerialPABotBase Messages (Nintendo Switch Wired Controller)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_NS_WiredController_H
#define PokemonAutomation_SerialPABotBase_Messages_NS_WiredController_H

#include "../ControllerStates/NintendoSwitch_WiredController_State.h"
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


#define PABB_MSG_COMMAND_NS_WIRED_CONTROLLER_STATE      0x90
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_NintendoSwitch_WiredController_State report;
} PABB_PACK pabb_Message_Command_NS_WiredController_State;



#ifdef __cplusplus
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
