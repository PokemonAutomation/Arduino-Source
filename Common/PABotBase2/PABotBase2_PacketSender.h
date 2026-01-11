/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionSender_H
#define PokemonAutomation_PABotBase2_ConnectionSender_H

#include "PABotBase2_Connection.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef PABB2_ConnectionSender_SLOTS
#define PABB2_ConnectionSender_SLOTS        8   //  Must be power-of-two, fits into uint8_t. (max 128)
#endif

#ifndef PABB2_ConnectionSender_BUFFER_SIZE
#define PABB2_ConnectionSender_BUFFER_SIZE  64  //  Must fit into size_t.
#endif

#ifndef PABB2_ConnectionSender_RETRANSMIT_COUNTER
#define PABB2_ConnectionSender_RETRANSMIT_COUNTER   2
#endif



#define PABB2_ConnectionSender_SLOTS_MASK   (uint8_t)((PABB2_ConnectionSender_SLOTS) - 1)


typedef void (*pabb2_fp_DataSender)(void* context, const void* data, size_t bytes);

typedef struct{
    uint8_t slot_head;
    uint8_t slot_tail;

    //  A timer that increments each time "pabb2_PacketSender_iterate_retransmits()"
    //  is called and nothing is re-transmitted.
    uint8_t retransmit_seqnum;

    //  If non-zero, it indicates we are in the middle of sending stream bytes.
    //  The current packet must be finished before sending anything else.
//    uint8_t pending_stream;

    uint16_t stream_offset;

    //  (buffer_head == buffer_tail) is ambiguous between empty and full.
    //  However, queue empty also has (slot_head == slot_tail).
    size_t buffer_head;
    size_t buffer_tail;

    pabb2_fp_DataSender sender;
    void* sender_context;

    //  These store the offsets within the buffer where the packet starts.
    //  The values stored here have their bits flipped so that zero means invalid.
    size_t offsets[PABB2_ConnectionSender_SLOTS];

    uint8_t buffer[PABB2_ConnectionSender_BUFFER_SIZE];
} pabb2_PacketSender;


//  Initialize/Construct the queue.
void pabb2_PacketSender_init(
    pabb2_PacketSender* self,
    pabb2_fp_DataSender sender,
    void* sender_context
);

inline uint8_t pabb2_PacketSender_size(pabb2_PacketSender* self){
    return self->slot_tail - self->slot_head;
}

//  Remove the packet corresponding to the specified seqnum from the queue.
//  Returns true is successful, false if seqnum is not in the queue.
bool pabb2_PacketSender_remove(pabb2_PacketSender* self, uint8_t seqnum);

//
//  Send a packet with the specified opcode and extra data after the header.
//  Return a pointer to the header if successful.
//  Returns NULL if queue is full.
//
const pabb2_PacketHeader* pabb2_PacketSender_send_packet(
    pabb2_PacketSender* self,
    uint8_t opcode, uint8_t extra_bytes, const void* extra_data
);

//
//  Send a single packet in two parts:
//      1.  Reserve a buffer to fill with the packet.
//      2.  Commit the packet to the queue.
//
//  On success, "reserve()" will return a pointer to the packet to be filled.
//  All fields of the packet header are already filled and should not be
//  modified by the user. The user should only populate the bytes that follow
//  the header.
//
//  On fail, reserve will return a null pointer. No changes are made to the
//  queue on failure. Failures will happen if there is insufficient space in
//  the queue.
//
//  "commit()" will add the packet to the queue. It will add the CRC checksum
//  so you don't need to do it yourself.
//
//  If "reserve()" succeeds, it must be followed by "commit()" before any other
//  function can be called on this queue. It is undefined behavior to separate
//  "reserve()" and "commit()" with another function on the same queue.
//
pabb2_PacketHeader* pabb2_PacketSender_reserve_packet(
    pabb2_PacketSender* self,
    uint8_t opcode, uint8_t extra_bytes
);
void pabb2_PacketSender_commit_packet(pabb2_PacketSender* self, pabb2_PacketHeader* packet);

//
//  Send the specified data on the data stream.
//  Returns the # of bytes actually sent.
//  Returning less than "bytes" indicates the queue is full.
//
size_t pabb2_PacketSender_send_stream(
    pabb2_PacketSender* self,
    const void* data, size_t bytes
);



bool pabb2_PacketSender_iterate_retransmits(pabb2_PacketSender* self);










#ifdef __cplusplus
}
#endif

#endif
