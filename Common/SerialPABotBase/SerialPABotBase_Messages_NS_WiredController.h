/*  SerialPABotBase Messages (Nintendo Switch Wired Controller)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_NS_WiredController_H
#define PokemonAutomation_SerialPABotBase_Messages_NS_WiredController_H

#include "../ControllerStates/NintendoSwitch_WiredController_State.h"
#include "SerialPABotBase_Protocol.h"

#pragma pack(push, 1)


#ifdef __cplusplus
extern "C" {
#endif


#define PABB_MSG_COMMAND_NS_WIRED_CONTROLLER_STATE      0x90
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_NintendoSwitch_WiredController_State report;
} pabb_Message_Command_NS_WiredController_State;



#ifdef __cplusplus
}
#endif


#pragma pack(pop)

#endif
