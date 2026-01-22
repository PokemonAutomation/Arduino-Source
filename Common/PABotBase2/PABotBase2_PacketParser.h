/*  PABotBase2 Packet Parser
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_PacketParser_H
#define PokemonAutomation_PABotBase2_PacketParser_H

#include "PABotBase2_StreamInterface.h"
#include "PABotBase2_Connection.h"

#ifdef __cplusplus
extern "C" {
#endif



//  Maximum size of incoming packet + overhead.
//  Min Size: sizeof(largest header) + sizeof(uint32_t) + 1
//  Max Size: 256
#ifndef PABB2_MAX_INCOMING_PACKET_SIZE
#define PABB2_MAX_INCOMING_PACKET_SIZE      ((uint16_t)20)
#endif



#define PABB2_PacketParser_RESULT_VALID             0
#define PABB2_PacketParser_RESULT_INVALID           1
#define PABB2_PacketParser_RESULT_CHECKSUM_FAIL     2


typedef struct{
    uint8_t index;
    uint8_t buffer[PABB2_MAX_INCOMING_PACKET_SIZE];
} pabb2_PacketParser;



inline void pabb2_PacketParser_init(pabb2_PacketParser* self){
    self->index = 0;
}
inline void pabb2_PacketParser_reset(pabb2_PacketParser* self){
    self->index = 0;
}

//
//  Parse packets by consuming data by pulling from a reader.
//
//  Returns NULL if more data is needed.
//  Returns the packet header on a completed/invalid packet.
//  (return_value->magic_number) indicates the status of the packet.
//
//  This will never read more than is necessary unless there
//  is an error.
//
const pabb2_PacketHeader* pabb2_PacketParser_pull_bytes(
    pabb2_PacketParser* self,
    void* context, pabb2_fp_StreamRecv recv
);

//
//  Parse packets by consuming data from an existing buffer.
//  This will consume all the bytes you give it and may call
//  "packet_runner" multiple times.
//
typedef void (*pabb2_fp_PacketRunner)(void* context, const pabb2_PacketHeader* data);
void pabb2_PacketParser_push_bytes(
    pabb2_PacketParser* self,
    void* context, pabb2_fp_PacketRunner packet_runner,
    const uint8_t* data, size_t bytes
);




#ifdef __cplusplus
}
#endif
#endif
