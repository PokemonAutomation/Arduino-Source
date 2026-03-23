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




#define PABB2_MESSAGE_PROTOCOL_VERSION      2026031600


#define PABB2_MESSAGE_OPCODE_INVALID            0x00


struct PABB_PACK pabb2_MessageHeader{
    uint16_t message_bytes;
    uint8_t opcode;
    uint8_t id;
};


#define PABB2_MESSAGE_OPCODE_RET                0x10
#define PABB2_MESSAGE_OPCODE_REQUEST_DROPPED    0x1f


#define PABB2_MESSAGE_OPCODE_RET_U32            0x11
struct PABB_PACK pabb2_Message_Response_u32{
    uint16_t message_bytes;
    uint8_t opcode;
    uint8_t id;
    uint32_t data;
};


#define PABB2_MESSAGE_OPCODE_RET_DATA           0x12
struct PABB_PACK pabb2_Message_Response_Data{
    uint16_t message_bytes;
    uint8_t opcode;
    uint8_t id;
};


#define PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION       0x20
#define PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION       0x21
#define PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER      0x22
#define PABB2_MESSAGE_OPCODE_DEVICE_NAME            0x23
#define PABB2_MESSAGE_OPCODE_CONTROLLER_LIST        0x24
#define PABB2_MESSAGE_OPCODE_CQ_CAPACITY            0x28

#define PABB_MESSAGE_OPCODE_READ_CONTROLLER_MODE    0x30
#define PABB_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE  0x31
#define PABB_MESSAGE_OPCODE_RESET_TO_CONTROLLER     0x32
#define PABB_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS  0x33
#define PABB_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS      0x34

#define PABB2_MESSAGE_OPCODE_CQ_CANCEL              0x40
#define PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT     0x41
#define PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED    0x42



}
}


#if _WIN32
#pragma pack(pop)
#endif


#endif
