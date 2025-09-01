/*  SerialPABotBase Messages (Nintendo Switch 1 Wireless Controllers)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_NS1_WirelessControllers_H
#define PokemonAutomation_SerialPABotBase_Messages_NS1_WirelessControllers_H

#include "../ControllerStates/NintendoSwitch_WirelessController_State.h"
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



typedef struct{
    uint8_t body[3];
    uint8_t buttons[3];
    uint8_t left_grip[3];
    uint8_t right_grip[3];
} PABB_NintendoSwitch_ControllerColors;





#define PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_READ_SPI   0x60
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} PABB_PACK pabb_Message_NS1_WirelessController_ReadSpi;

#define PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_WRITE_SPI  0x61
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} PABB_PACK pabb_Message_NS1_WirelessController_WriteSpi;



#define PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_BUTTONS    0xa0
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons;
} PABB_PACK pabb_Message_Command_NS1_WirelessController_Buttons;


#define PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_FULL_STATE 0xa1
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pabb_NintendoSwitch_WirelessController_State0x30 state;
} PABB_PACK pabb_Message_Command_NS1_WirelessController_FullState;




#ifdef __cplusplus
}
#endif



#if _WIN32
#pragma pack(pop)
#endif


#endif
