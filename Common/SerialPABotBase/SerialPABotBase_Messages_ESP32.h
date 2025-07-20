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

typedef struct{
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t left_joystick[3];
    uint8_t right_joystick[3];
    uint8_t vibrator;
} PABB_NintendoSwitch_ButtonState;

typedef struct{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t rotation_x;
    int16_t rotation_y;
    int16_t rotation_z;
} PABB_NintendoSwitch_GyroState;

typedef struct{
    PABB_NintendoSwitch_GyroState time0;
    PABB_NintendoSwitch_GyroState time1;
    PABB_NintendoSwitch_GyroState time2;
} PABB_NintendoSwitch_GyroStateX3;





#define PABB_MSG_ESP32_REQUEST_READ_SPI         0x60
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} PABB_PACK pabb_Message_ESP32_ReadSpi;

#define PABB_MSG_ESP32_REQUEST_WRITE_SPI        0x61
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_type;
    uint32_t address;
    uint8_t bytes;
} PABB_PACK pabb_Message_ESP32_WriteSpi;



#define PABB_MSG_ESP32_CONTROLLER_STATE_BUTTONS     0xa0
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    PABB_NintendoSwitch_ButtonState buttons;
} PABB_PACK pabb_Message_ESP32_CommandButtonState;


#define PABB_MSG_ESP32_CONTROLLER_STATE_FULL        0xa1
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    PABB_NintendoSwitch_ButtonState buttons;
    PABB_NintendoSwitch_GyroStateX3 gyro;
} PABB_PACK pabb_Message_ESP32_CommandFullState;




#ifdef __cplusplus
}
#endif



#if _WIN32
#pragma pack(pop)
#endif


#endif
