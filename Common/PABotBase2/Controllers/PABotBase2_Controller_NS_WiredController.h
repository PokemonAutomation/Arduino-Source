/*  PABotBase2 NS Wired Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_Controller_NS_WiredController_H
#define PokemonAutomation_PABotBase2_Controller_NS_WiredController_H

#include "../../ControllerStates/NintendoSwitch_WiredController_State.h"
#include "../PABotBase2_MessageProtocol.h"

#pragma pack(push, 1)

namespace PokemonAutomation{
namespace PABotBase2{



#define PABB2_MESSAGE_CMD_NS_WIRED_CONTROLLER_STATE     0x90
struct pabb2_Message_Command_NS_WiredController_State : MessageHeader{
    uint16_t milliseconds;
    pabb_NintendoSwitch_WiredController_State report;
};




}
}



#pragma pack(pop)


#endif
