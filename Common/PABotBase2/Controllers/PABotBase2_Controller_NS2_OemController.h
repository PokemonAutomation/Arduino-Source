/*  PABotBase2 NS2 OEM Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_Controller_NS2_OemController_H
#define PokemonAutomation_PABotBase2_Controller_NS2_OemController_H

#include "../../ControllerStates/NintendoSwitch2_OemController_State.h"
#include "../PABotBase2_MessageProtocol.h"

#pragma pack(push, 1)

namespace PokemonAutomation{
namespace PABotBase2{



//
//  Report format
//
//  Each S2 target CID presents its real controller's input report format,
//  matching the real controller's behavior:
//
//    PABB_CID_NintendoSwitch2_WiredController    (0x1010, grip/combined)  0x05
//    PABB_CID_NintendoSwitch2_WiredProController / _WirelessProController  0x09
//    PABB_CID_NintendoSwitch2_WiredLeftJoycon / _WirelessLeftJoycon       0x07
//    PABB_CID_NintendoSwitch2_WiredRightJoycon / _WirelessRightJoycon     0x08
//
//  (The NSO GameCube's 0x0A is available to a future GC CID.) The target
//  streams that format by default and implements 0x03/0x0A so the host
//  can switch it to 0x05, as a real S2 does. The serial messages below
//  carry the report minus the fields the firmware fills in (the report
//  counter, the power-info byte on the native reports, and the USB
//  report ID byte) - the CC fills the matching state struct and the
//  firmware passes those bytes through without translation. The 40-byte
//  native motion payload is bit-packed with a different codec per format,
//  so it is generated on the CC side.
//


//
//  Flash access on the physical S2 (S2 command 0x02).
//
//  The controller allows 0x50-byte reads over USB and 0x4F over BLE; the
//  72-byte serial message cap limits the payload to 68 bytes, so the CC
//  steps the address for longer regions.
//


#define PABB2_MESSAGE_REQ_NS2_OEM_CONTROLLER_READ_FLASH      0xa0
#define PABB2_MESSAGE_REQ_NS2_OEM_CONTROLLER_WRITE_FLASH     0xa1
struct Message_NS2_OemController_Flash : MessageHeader{
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
};


//
//  Target output
//


//  Echoed to the CC when the game writes the target's rumble output
//  report (report 0x01 / 0x02 / 0x03). The GC's 4-byte motor and the
//  Joy-Con 2's single 16-byte LRA frame both fit in the state struct.
#define PABB2_MESSAGE_INFO_NS2_OEM_CONTROLLER_RUMBLE            0xa2
struct Message_Feedback_NS2_OemController_Rumble : MessageHeader{
    NintendoSwitch2::OemController_RumbleState data;
};


//
//  Target input
//
//  Per report format: a fast-path buttons message and a full-state
//  message. The buttons struct is an exact prefix of the full state
//  (buttons + sticks + the format's unknown byte(s), and the GameCube
//  triggers where the 0x0A format has them contiguously), so the firmware
//  updates the cached state with a single memcpy. The full-state messages
//  are 65/67 bytes: they fit the 72-byte incoming message cap and, at the
//  96 byte packet cap, travel as a single RSC packet.
//


#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X05       0xa3
struct Message_Command_NS2_OemController_Buttons0x05 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x05_Buttons buttons;
};

#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X05    0xa4
struct Message_Command_NS2_OemController_FullState0x05 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x05 state;
};


#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X07       0xa5
struct Message_Command_NS2_OemController_Buttons0x07 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x07_Buttons buttons;
};

#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X07    0xa6
struct Message_Command_NS2_OemController_FullState0x07 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x07 state;
};


#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X08       0xa7
struct Message_Command_NS2_OemController_Buttons0x08 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x08_Buttons buttons;
};

#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X08    0xa8
struct Message_Command_NS2_OemController_FullState0x08 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x08 state;
};


#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X09       0xa9
struct Message_Command_NS2_OemController_Buttons0x09 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x09_Buttons buttons;
};

#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X09    0xaa
struct Message_Command_NS2_OemController_FullState0x09 : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x09 state;
};


#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS_0X0A       0xab
struct Message_Command_NS2_OemController_Buttons0x0A : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x0A_Buttons buttons;
};

#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE_0X0A    0xac
struct Message_Command_NS2_OemController_FullState0x0A : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x0A state;
};



}
}

#pragma pack(pop)

#endif
