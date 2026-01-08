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


#define PABB2_CONNECTION_PACKET_MAGIC_NUMBER                0x81

typedef struct{
    uint8_t magic_number;
    uint8_t opcode;
    uint16_t packet_bytes;
} pabb2_PacketHeader;

#define PABB2_CONNECTION_PACKET_OPCODE_INVALID              0x00
#define PABB2_CONNECTION_PACKET_OPCODE_RESET                0x01

#define PABB2_CONNECTION_PACKET_OPCODE_REQUEST_PACKET_SIZE  0x10
#define PABB2_CONNECTION_PACKET_OPCODE_REQUEST_BUFFER_SIZE  0x11



typedef struct{
    uint8_t magic_number;
    uint8_t opcode;
    uint16_t packet_bytes;
    uint32_t stream_offset;
} pabb2_PacketHeaderData;

#define PABB2_CONNECTION_PACKET_OPCODE_REQUEST_DATA         0x20
#define PABB2_CONNECTION_PACKET_OPCODE_SEND_DATA            0x90



#ifdef __cplusplus
}
#endif

#if _WIN32
#pragma pack(pop)
#endif

#endif
