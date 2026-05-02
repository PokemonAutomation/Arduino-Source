/*  PABotBase2 NS1 OEM Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_Controller_NS1_OemController_H
#define PokemonAutomation_PABotBase2_Controller_NS1_OemController_H

#include "../../ControllerStates/NintendoSwitch_OemController_State.h"
#include "../PABotBase2_MessageProtocol.h"

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


struct PABB_NintendoSwitch_ControllerColors{
    uint8_t body[3];
    uint8_t buttons[3];
    uint8_t left_grip[3];
    uint8_t right_grip[3];
};


#define PABB2_MESSAGE_REQ_NS1_OEM_CONTROLLER_READ_SPI        0x91
#define PABB2_MESSAGE_REQ_NS1_OEM_CONTROLLER_WRITE_SPI       0x92
struct PABB_PACK pabb2_Message_NS1_OemController_Spi : MessageHeader{
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
};


#define PABB2_MESSAGE_REQ_NS1_OEM_CONTROLLER_PLAYER_LIGHTS      0x93
#define PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_PLAYER_LIGHTS     0x94
#define PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED    0x95
#define PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_RUMBLE            0x96
struct PABB_PACK pabb2_Message_Feedback_NS1_OemController_Rumble : MessageHeader{
    pabb_NintendoSwitch_Rumble data;
};


#define PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_BUTTONS            0x97
struct PABB_PACK pabb2_Message_Command_NS1_OemController_Buttons : MessageHeader{
    uint16_t milliseconds;
    pabb_NintendoSwitch_OemController_State0x30_Buttons buttons;
};


#define PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_FULL_STATE         0x98
struct PABB_PACK pabb2_Message_Command_NS1_OemController_FullState : MessageHeader{
    uint16_t milliseconds;
    pabb_NintendoSwitch_OemController_State0x30 state;
};



}
}



#if _WIN32
#pragma pack(pop)
#endif


#endif
