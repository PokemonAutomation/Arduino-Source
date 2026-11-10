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
//  The S1 normalized all of its OEM targets to report 0x30; the S2
//  normalizes to report 0x05 - the one input report format every S2
//  controller type can produce (the "grip" format). The S2 host selects
//  it on the physical controller (command 0x03 sub 0x0A over USB, the
//  0x000a characteristic over BLE), so the 0x05 report bytes pass through
//  the firmware unmodified from the physical controller to the target.
//


//
//  Flash access on the physical S2 (S2 command 0x02).
//
//  Reads cap at 0x50 bytes over USB and 0x4F over BLE; the handler
//  enforces the smaller of the two for the session in use.
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
//  Note: the full-state message is 69 bytes (4 header + 2 ms + 63 report),
//  which is above the current PABB2_MAX_INCOMING_MESSAGE_SIZE of 64 - raise
//  the board cap when the S2 targets are wired in.
//


//  Fast path: buttons + sticks + GameCube triggers (12 bytes). The
//  remaining 0x05 fields (counter, optical, magnetometer, battery, motion)
//  are untouched by this command.
#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_BUTTONS            0xa3
struct Message_Command_NS2_OemController_Buttons : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x05_Buttons buttons;
};


#define PABB2_MESSAGE_CMD_NS2_OEM_CONTROLLER_FULL_STATE         0xa4
struct Message_Command_NS2_OemController_FullState : MessageHeader{
    uint16_t milliseconds;
    NintendoSwitch2::OemController_State0x05 state;
};



}
}

#pragma pack(pop)

#endif
