/*  HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_Controller_HID_Keyboard_H
#define PokemonAutomation_PABotBase2_Controller_HID_Keyboard_H

#include "../../ControllerStates/StandardHid_Keyboard_State.h"
#include "../PABotBase2_MessageProtocol.h"

#pragma pack(push, 1)

namespace PokemonAutomation{
namespace PABotBase2{



#define PABB2_MESSAGE_CMD_HID_KEYBOARD_STATE            0x82
struct pabb2_Message_Command_HID_Keyboard_State : MessageHeader{
    uint16_t milliseconds;
    pabb_HID_Keyboard_State report;
};




}
}



#pragma pack(pop)


#endif
