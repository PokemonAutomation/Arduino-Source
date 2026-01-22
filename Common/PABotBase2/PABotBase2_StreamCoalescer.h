/*  PABotBase2 Stream Coalescer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_StreamCoalescer_H
#define PokemonAutomation_PABotBase2_StreamCoalescer_H

#include "PABotBase2_Connection.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef PABB2_StreamCoalescer_SLOTS
#define PABB2_StreamCoalescer_SLOTS         4   //  Must be power-of-two, fits into uint8_t. (< 256)
#endif

#ifndef PABB2_StreamCoalescer_BUFFER_SIZE
#define PABB2_StreamCoalescer_BUFFER_SIZE   64  //  Must be power-of-two, fits into size_t.
#endif



#define PABB2_StreamCoalescer_SLOTS_MASK        (uint8_t)((PABB2_StreamCoalescer_SLOTS) - 1)
#define PABB2_StreamCoalescer_BUFFER_MASK       (size_t)((PABB2_StreamCoalescer_BUFFER_SIZE) - 1)


typedef struct{
    uint8_t slot_head;
    uint8_t slot_tail;

//    uint8_t seqnum;
    uint16_t stream_head;
    uint16_t stream_tail;

    bool stream_reset;

    //  0       =   Not received yet.
    //  0-254   =   Received stream packet. # is the size.
    //  255     =   Received non-stream packet.
    uint8_t lengths[PABB2_StreamCoalescer_SLOTS];

    uint16_t offsets[PABB2_StreamCoalescer_SLOTS];

    uint8_t buffer[PABB2_StreamCoalescer_BUFFER_SIZE];
} pabb2_StreamCoalescer;


void pabb2_StreamCoalescer_init(pabb2_StreamCoalescer* self);
inline void pabb2_StreamCoalescer_reset(pabb2_StreamCoalescer* self){
    pabb2_StreamCoalescer_init(self);
    self->stream_reset = true;
}

void pabb2_StreamCoalescer_push_packet(pabb2_StreamCoalescer* self, uint8_t seqnum);

//  Returns true if the packet has been received and can be acked.
bool pabb2_StreamCoalescer_push_stream(pabb2_StreamCoalescer* self, const pabb2_PacketHeaderData* packet);

//
//  Read data from the stream.
//
//  Returns the # of bytes actually read.
//  Returns (size_t)-1 if the stream has been reset.
//  Returning less than "max_bytes" indicates the stream is out of usable data.
//
size_t pabb2_StreamCoalescer_read(pabb2_StreamCoalescer* self, void* data, size_t max_bytes);




#ifdef __cplusplus
}
#endif
#endif
