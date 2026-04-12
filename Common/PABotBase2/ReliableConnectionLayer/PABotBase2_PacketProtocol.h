/*  PABotBase2 Packet Protocol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ReliableConnectionLayer_PacketProtocol_H
#define PokemonAutomation_PABotBase2_ReliableConnectionLayer_PacketProtocol_H

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


//#define PABB2_CONNECTION_BAUD_RATE                  115200
#define PABB2_CONNECTION_BAUD_RATE                  460800
#define PABB2_CONNECTION_MAGIC_NUMBER               0x81
#define PABB2_CONNECTION_PROTOCOL_VERSION           2026041100


#define PABB2_CONNECTION_RETRANSMIT_FLAG            0x80
#define PABB2_CONNECTION_OPCODE_MASK                0x7f


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
#define PABB2_CONNECTION_OPCODE_RET_RESET           0x41


struct PABB_PACK PacketHeader_u8{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint8_t data;
};
struct PABB_PACK PacketHeader_u16{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t data;
};
struct PABB_PACK PacketHeader_u32{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint32_t data;
};



//
//  Requests (acks required)
//

#define PABB2_CONNECTION_OPCODE_ASK_VERSION         0x02
#define PABB2_CONNECTION_OPCODE_RET_VERSION         0x42

#define PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE     0x03
#define PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE     0x43

#define PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS    0x04
#define PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS    0x44

#define PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES    0x05
#define PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES    0x45

//#define PABB2_CONNECTION_OPCODE_ASK_STREAM_READY    0x10
//#define PABB2_CONNECTION_OPCODE_RET_STREAM_READY    0x50
#define PABB2_CONNECTION_OPCODE_INFO_STREAM_DEAD    0x10
#define PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA     0x12
#define PABB2_CONNECTION_OPCODE_RET_STREAM_DATA     0x52
#define PABB2_CONNECTION_OPCODE_ASK_STREAM_REQUEST  0x13    //  Unused for now.
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

#define PABB2_CONNECTION_OPCODE_INFO                    0x20
#define PABB2_CONNECTION_OPCODE_INFO_U8                 0x21
#define PABB2_CONNECTION_OPCODE_INFO_U16                0x22
#define PABB2_CONNECTION_OPCODE_INFO_H32                0x23
#define PABB2_CONNECTION_OPCODE_INFO_U32                0x24
#define PABB2_CONNECTION_OPCODE_INFO_I32                0x25
#define PABB2_CONNECTION_OPCODE_INFO_BINARY             0x26
#define PABB2_CONNECTION_OPCODE_INFO_STR                0x27
#define PABB2_CONNECTION_OPCODE_INFO_LABEL_H32          0x28
#define PABB2_CONNECTION_OPCODE_INFO_LABEL_U32          0x29
#define PABB2_CONNECTION_OPCODE_INFO_LABEL_I32          0x2a

#define PABB2_CONNECTION_OPCODE_INVALID_LENGTH          0x30
#define PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL   0x31
#define PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE          0x32



}
}

#if _WIN32
#pragma pack(pop)
#endif

#endif
