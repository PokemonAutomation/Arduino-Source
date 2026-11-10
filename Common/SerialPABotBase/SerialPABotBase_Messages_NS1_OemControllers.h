/*  SerialPABotBase Messages (Nintendo Switch 1 OEM Controllers)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_NS1_OemControllers_H
#define PokemonAutomation_SerialPABotBase_Messages_NS1_OemControllers_H

#include "../ControllerStates/NintendoSwitch_OemController_State.h"
#include "SerialPABotBase_Protocol.h"

#pragma pack(push, 1)


#ifdef __cplusplus
extern "C" {
#endif



typedef struct{
    uint8_t body[3];
    uint8_t buttons[3];
    uint8_t left_grip[3];
    uint8_t right_grip[3];
} PABB_NintendoSwitch_ControllerColors;





#define PABB_MSG_INFO_NS1_OEM_CONTROLLER_RUMBLE             0x30
typedef struct{
    OemController_RumbleState rumble;
} pabb_MsgInfo_NS1_OemController_Rumble;

#define PABB_MSG_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED     0x31



#define PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_READ_SPI        0x60
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} pabb_Message_NS1_OemController_ReadSpi;

#define PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_WRITE_SPI       0x61
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} pabb_Message_NS1_OemController_WriteSpi;

#define PABB_MSG_REQUEST_NS1_OEM_CONTROLLER_PLAYER_LIGHTS   0x62
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
} pabb_MsgInfo_NS1_OemController_PlayerLights;



#define PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_BUTTONS         0xa0
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_NintendoSwitch_OemController_State0x30_Buttons buttons;
} pabb_Message_Command_NS1_OemController_Buttons;


#define PABB_MSG_COMMAND_NS1_OEM_CONTROLLER_FULL_STATE      0xa1
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_NintendoSwitch_OemController_State0x30 state;
} pabb_Message_Command_NS1_OemController_FullState;




#ifdef __cplusplus
}
#endif



#pragma pack(pop)


#endif
