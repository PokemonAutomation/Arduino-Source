/*  ESP32 Commands
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ESP32_H
#define PokemonAutomation_NintendoSwitch_ESP32_H

#include "Common/Microcontroller/MessageProtocol.h"

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif


#ifdef __cplusplus
namespace PokemonAutomation{
namespace NintendoSwitch{
#endif


typedef struct{
    uint8_t report_id;
    uint8_t timer;
    uint8_t byte2;
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t leftstick_x_lo;
    uint8_t leftstick_x_hi;
    uint8_t leftstick_y;
    uint8_t rightstick_x_lo;
    uint8_t rightstick_x_hi;
    uint8_t rightstick_y;
    uint8_t vibrator;
    uint8_t gyro[49 - 13];
} PABB_PACK ESP32Report0x30;

#define PABB_MSG_ESP32_REPORT       0x9e
typedef struct{
    seqnum_t seqnum;
    uint8_t ticks;
    ESP32Report0x30 report;
} PABB_PACK pabb_esp32_report30;




#ifdef __cplusplus
}
}
#endif



#if _WIN32
#pragma pack(pop)
#endif


#endif
