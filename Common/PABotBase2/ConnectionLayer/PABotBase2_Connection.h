/*  PABotBase2 Connection Protocol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_ConnectionProtocol_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_ConnectionProtocol_H

#include <stdbool.h>
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


#define PABB2_CONNECTION_BAUD_RATE                  115200
#define PABB2_CONNECTION_MAGIC_NUMBER               0x81
#define PABB2_CONNECTION_PROTOCOL_VERSION           2025011700


//
//  Special
//

struct PABB_PACK PacketHeader{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
};

#define PABB2_CONNECTION_OPCODE_INVALID             0x00

#define PABB2_CONNECTION_OPCODE_ASK_RESET           0x01
#define PABB2_CONNECTION_OPCODE_RET_RESET           0x81


struct PABB_PACK PacketHeader_Ack_u8{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint8_t data;
};
struct PABB_PACK PacketHeader_Ack_u16{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t data;
};
struct PABB_PACK PacketHeader_Ack_u32{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint32_t data;
};



//
//  Requests (acks required)
//

#define PABB2_CONNECTION_OPCODE_ASK_VERSION         0x20
#define PABB2_CONNECTION_OPCODE_RET_VERSION         0xa0

#define PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE     0x21
#define PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE     0xa1

#define PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS    0x22
#define PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS    0xa2

#define PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES    0x23
#define PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES    0xa3

#define PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA     0x30
#define PABB2_CONNECTION_OPCODE_RET_STREAM_DATA     0xb0
#define PABB2_CONNECTION_OPCODE_ASK_STREAM_REQUEST  0x31
struct PABB_PACK PacketHeaderData{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t stream_offset;
};



//
//  Out-of-band Info (no acks needed)
//

#define PABB2_CONNECTION_OPCODE_INFO                    0x40
#define PABB2_CONNECTION_OPCODE_INFO_U8                 0x41
#define PABB2_CONNECTION_OPCODE_INFO_U16                0x42
#define PABB2_CONNECTION_OPCODE_INFO_U32                0x43

#define PABB2_CONNECTION_OPCODE_INVALID_LENGTH          0x60
#define PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL   0x61
#define PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE          0x62



}
}

#if _WIN32
#pragma pack(pop)
#endif

#endif
