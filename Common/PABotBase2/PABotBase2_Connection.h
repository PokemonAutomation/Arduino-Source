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


#define PABB2_CONNECTION_MAGIC_NUMBER                   0x81
#define PABB2_CONNECTION_PROTOCOL_VERSION               2023011700


//
//  Special
//

typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
} PABB_PACK pabb2_PacketHeader;

#define PABB2_CONNECTION_OPCODE_INVALID                 0x00
#define PABB2_CONNECTION_OPCODE_RESET                   0x01


typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t data;
} PABB_PACK pabb2_PacketHeader_Ack_u16;
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint8_t data;
} PABB_PACK pabb2_PacketHeader_Ack_u8;
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint32_t data;
} PABB_PACK pabb2_PacketHeader_Ack_u32;



//
//  Requests (acks required)
//

#define PABB2_CONNECTION_OPCODE_ASK_VERSION         0x20
#define PABB2_CONNECTION_OPCODE_RET_VERSION         0x40

#define PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE     0x21
#define PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE     0x41

#define PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS    0x22
#define PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS    0x42

#define PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES    0x23
#define PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES    0x43

#define PABB2_CONNECTION_OPCODE_STREAM_DATA         0x30
#define PABB2_CONNECTION_OPCODE_STREAM_REQUEST      0x31
typedef struct{
    uint8_t magic_number;
    uint8_t seqnum;
    uint8_t packet_bytes;
    uint8_t opcode;
    uint16_t stream_offset;
} PABB_PACK pabb2_PacketHeaderData;




//
//  Acks
//




#define PABB2_CONNECTION_OPCODE_RET                     0x50
#define PABB2_CONNECTION_OPCODE_RET_u8                  0x51
#define PABB2_CONNECTION_OPCODE_RET_u16                 0x52
#define PABB2_CONNECTION_OPCODE_RET_u32                 0x53



//
//  Out-of-band Info (no acks needed)
//

#define PABB2_CONNECTION_OPCODE_INVALID_LENGTH          0x60
#define PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL   0x61
#define PABB2_CONNECTION_OPCODE_INVALID_OPCODE          0x62
#define PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE          0x63




#ifdef __cplusplus
}
#endif

#if _WIN32
#pragma pack(pop)
#endif

#endif
