/*  PABotBase2 Connection Protocol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionProtocol_H
#define PokemonAutomation_PABotBase2_ConnectionProtocol_H

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

#ifdef __cplusplus
extern "C" {
#endif


#define PABB2_CONNECTION_PACKET_MAGIC_NUMBER                    0x81


//
//  Special
//

typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
} pabb2_PacketHeader;

#define PABB2_CONNECTION_PACKET_OPCODE_INVALID                  0x00
#define PABB2_CONNECTION_PACKET_OPCODE_RESET                    0x01



//
//  Requests (acks required)
//

#define PABB2_CONNECTION_PACKET_OPCODE_QUERY_PACKET_SIZE        0x20
#define PABB2_CONNECTION_PACKET_OPCODE_QUERY_BUFFER_SLOTS       0x21
#define PABB2_CONNECTION_PACKET_OPCODE_QUERY_BUFFER_BYTES       0x22

#define PABB2_CONNECTION_PACKET_OPCODE_STREAM_DATA              0x30
#define PABB2_CONNECTION_PACKET_OPCODE_STREAM_REQUEST           0x31
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t stream_offset;
} pabb2_PacketHeaderData;




//
//  Acks
//

#define PABB2_CONNECTION_PACKET_OPCODE_ACK                      0x40

#define PABB2_CONNECTION_PACKET_OPCODE_ACK_u8                   0x41
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint8_t data;
} pabb2_PacketHeader_Ack_u8;

#define PABB2_CONNECTION_PACKET_OPCODE_ACK_u16                  0x42
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t data;
} pabb2_PacketHeader_Ack_u16;

#define PABB2_CONNECTION_PACKET_OPCODE_ACK_u32                  0x43
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint32_t data;
} pabb2_PacketHeader_Ack_u32;



//
//  Out-of-band Info (no acks needed)
//

#define PABB2_CONNECTION_PACKET_OPCODE_INVALID_LENGTH           0x60
#define PABB2_CONNECTION_PACKET_OPCODE_INVALID_CHECKSUM_FAIL    0x61
#define PABB2_CONNECTION_PACKET_OPCODE_INVALID_OPCODE           0x62
#define PABB2_CONNECTION_PACKET_OPCODE_UNKNOWN_OPCODE           0x63




#ifdef __cplusplus
}
#endif

#if _WIN32
#pragma pack(pop)
#endif

#endif
