/*  PABotBase2 Message Protocol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_MessageProtocol_H
#define PokemonAutomation_PABotBase2_MessageProtocol_H

#include <stdint.h>

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



#define PABB2_MESSAGE_PROTOCOL_VERSION      2026041105


struct PABB_PACK MessageHeader{
    uint16_t message_bytes;
    uint8_t opcode;
    uint8_t id;
};
struct PABB_PACK Message_u32 : MessageHeader{
    uint32_t data;
};

#define PABB2_MESSAGE_OPCODE_INVALID                0x00
#define PABB2_MESSAGE_OPCODE_LOG_STRING             0x01
#define PABB2_MESSAGE_OPCODE_LOG_LABEL_H32          0x02
#define PABB2_MESSAGE_OPCODE_LOG_LABEL_U32          0x03
#define PABB2_MESSAGE_OPCODE_LOG_LABEL_I32          0x04

#define PABB2_MESSAGE_OPCODE_REQUEST_DROPPED        0x10
#define PABB2_MESSAGE_OPCODE_RET                    0x11
#define PABB2_MESSAGE_OPCODE_RET_U32                0x12
#define PABB2_MESSAGE_OPCODE_RET_DATA               0x13

#define PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION       0x20
#define PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION       0x21
#define PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER      0x22
#define PABB2_MESSAGE_OPCODE_DEVICE_NAME            0x23
#define PABB2_MESSAGE_OPCODE_CONTROLLER_LIST        0x24
#define PABB2_MESSAGE_OPCODE_SET_LOGGING_FLAG       0x25
#define PABB2_MESSAGE_OPCODE_CQ_CAPACITY            0x28

#define PABB2_MESSAGE_OPCODE_READ_CONTROLLER_MODE   0x30
#define PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE 0x31
#define PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER    0x32
#define PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS 0x33
#define PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS     0x34
#define PABB2_MESSAGE_OPCODE_REQUEST_STATUS         0x35
#define PABB2_MESSAGE_OPCODE_CONSOLE_DISCONNECT     0x36

#define PABB2_MESSAGE_OPCODE_CQ_COMMAND_DROPPED     0x40
#define PABB2_MESSAGE_OPCODE_CQ_CANCEL              0x41
#define PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT     0x42
#define PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED    0x43


}
}


#if _WIN32
#pragma pack(pop)
#endif


#endif
