/*  SerialPABotBase Messages (ESP32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_ESP32_H
#define PokemonAutomation_SerialPABotBase_Messages_ESP32_H

#include "SerialPABotBase_Protocol.h"

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif


#ifdef __cplusplus
extern "C" {
namespace PokemonAutomation{
namespace SerialPABotBase{
#endif



typedef struct{
    uint8_t body[3];
    uint8_t buttons[3];
    uint8_t left_grip[3];
    uint8_t right_grip[3];
} NintendoSwitch_ControllerColors;

typedef struct{
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t left_joystick[3];
    uint8_t right_joystick[3];
    uint8_t vibrator;
} NintendoSwitch_ButtonState;

typedef struct{
    uint8_t data[36];
} NintendoSwitch_GyroState;






#define PABB_MSG_ESP32_REQUEST_STATUS   0x50
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_Message_ESP32_RequestStatus;


#if 0
#define PABB_MSG_ESP32_SET_MAC_ADDRESS      0x61
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint8_t mac_address[6];
} PABB_PACK pabb_Message_ESP32_SetMacAddress;
#endif



#define PABB_MSG_ESP32_REQUEST_GET_COLORS       0x64
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
} PABB_PACK pabb_Message_ESP32_GetColors;

#define PABB_MSG_ESP32_REQUEST_SET_COLORS       0x65
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    NintendoSwitch_ControllerColors colors;
} PABB_PACK pabb_Message_ESP32_SetColors;


#define PABB_MSG_ESP32_CONTROLLER_STATE_BUTTONS     0xa0
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    NintendoSwitch_ButtonState buttons;
} PABB_PACK pabb_Message_ESP32_CommandButtonState;


#define PABB_MSG_ESP32_CONTROLLER_STATE_FULL        0xa1
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    NintendoSwitch_ButtonState buttons;
    NintendoSwitch_GyroState gyro;
} PABB_PACK pabb_Message_ESP32_CommandFullState;




//  Deprecated

typedef struct{
    uint8_t report_id;
    uint8_t timer;
    uint8_t byte2;
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t left_joystick[3];
    uint8_t right_joystick[3];
    uint8_t vibrator;
    uint8_t gyro[49 - 13];
} NintendoSwitch_ESP32Report0x30;


#define PABB_MSG_ESP32_REPORT       0x9e
typedef struct{
    seqnum_t seqnum;
    uint8_t ticks;
    bool active;
    NintendoSwitch_ESP32Report0x30 report;
} PABB_PACK pabb_esp32_report30;




#ifdef __cplusplus
}
}
}
#endif



#if _WIN32
#pragma pack(pop)
#endif


#endif
