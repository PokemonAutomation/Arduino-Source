/*  PABotBase2 NS1 OEM Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_Controller_NS1_OemController_H
#define PokemonAutomation_PABotBase2_Controller_NS1_OemController_H

#include "../../ControllerStates/NintendoSwitch_OemController_State.h"

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



#define PABB_MSG_REQ_NS1_OEM_CONTROLLER_READ_SPI        0x91
#define PABB_MSG_REQ_NS1_OEM_CONTROLLER_WRITE_SPI       0x92
typedef struct{
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} PABB_PACK pabb2_Message_NS1_OemController_Spi;


#define PABB2_MESSAGE_REQ_NS1_OEM_CONTROLLER_PLAYER_LIGHTS      0x93
#define PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_PLAYER_LIGHTS     0x94
#define PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_USB_DISALLOWED    0x95
#define PABB2_MESSAGE_INFO_NS1_OEM_CONTROLLER_RUMBLE            0x96


#define PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_BUTTONS            0x97
typedef struct{
    uint16_t milliseconds;
    pabb_NintendoSwitch_OemController_State0x30_Buttons buttons;
} PABB_PACK pabb2_Message_Command_NS1_OemController_Buttons;


#define PABB2_MESSAGE_CMD_NS1_OEM_CONTROLLER_FULL_STATE         0x98
typedef struct{
    uint16_t milliseconds;
    pabb_NintendoSwitch_OemController_State0x30 state;
} PABB_PACK pabb2_Message_Command_NS1_OemController_FullState;



#ifdef __cplusplus
}
#endif



#if _WIN32
#pragma pack(pop)
#endif


#endif
