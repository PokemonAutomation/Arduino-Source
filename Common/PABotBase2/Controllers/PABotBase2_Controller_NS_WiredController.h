/*  PABotBase2 NS Wired Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_Controller_NS_WiredController_H
#define PokemonAutomation_PABotBase2_Controller_NS_WiredController_H

#include "../../ControllerStates/NintendoSwitch_WiredController_State.h"

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#elif __GNUC__
#define PABB_PACK   __attribute__((packed))
#else
#define PABB_PACK
#endif

namespace PokemonAutomation{
namespace PABotBase2{



#define PABB2_MESSAGE_CMD_NS_WIRED_CONTROLLER_STATE     0x90
struct PABB_PACK pabb2_Message_Command_NS_WiredController_State{
    uint16_t milliseconds;
    pabb_NintendoSwitch_WiredController_State report;
};




}
}



#if _WIN32
#pragma pack(pop)
#endif


#endif
