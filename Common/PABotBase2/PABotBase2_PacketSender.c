/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_PacketSender.h"


void pabb2_PacketSender_init(
    pabb2_PacketSender* self,
    pabb2_fp_DataSender sender,
    void* sender_context
){
    self->slot_head = 0;
    self->slot_tail = 0;
    self->retransmit_seqnum = 0;
//    self->pending_stream = 0;
    self->stream_offset = 0;
    self->buffer_head = 0;
    self->buffer_tail = 0;
    self->sender = sender;
    self->sender_context = sender_context;
    memset(self->offsets, 0, sizeof(self->offsets));
}

bool pabb2_PacketSender_remove(pabb2_PacketSender* self, uint8_t seqnum){
    {
        size_t offset = self->offsets[seqnum & PABB2_ConnectionSender_SLOTS_MASK];
        if (offset == 0){
            return false;
        }
        offset = ~offset;
        pabb2_PacketHeader* packet = (pabb2_PacketHeader*)(self->buffer + offset);
        packet->opcode = PABB2_CONNECTION_PACKET_OPCODE_INVALID;
    }

    //  Not the front.
    if (seqnum != self->slot_head){
        return true;
    }

    //  Clear out all acked requests from the front of the queue.
    while (true){
        self->offsets[seqnum & PABB2_ConnectionSender_SLOTS_MASK] = 0;
        seqnum++;
        self->slot_head = seqnum;

        //  Queue is empty.
        if (seqnum == self->slot_tail){
            self->buffer_head = 0;
            self->buffer_tail = 0;
            return true;
        }

        size_t offset = ~self->offsets[seqnum & PABB2_ConnectionSender_SLOTS_MASK];
        self->buffer_head = offset;

        pabb2_PacketHeader* packet = (pabb2_PacketHeader*)(self->buffer + offset);

        //  Packet hasn't been acked yet. We can't go further.
        if (packet->opcode != PABB2_CONNECTION_PACKET_OPCODE_INVALID){
            return true;
        }
    }
}

pabb2_PacketHeader* pabb2_PacketSender_reserve_packet(pabb2_PacketSender* self, uint8_t bytes){
    //  No slots available.
    uint8_t slots_used = self->slot_tail - self->slot_head;
    if (slots_used == PABB2_ConnectionSender_SLOTS){
        return NULL;
    }

    size_t buffer_head = self->buffer_head;
    size_t buffer_tail = self->buffer_tail;

    size_t capacity;
    size_t offset;

    if (slots_used == 0){
        //  We're empty. Rewind both to zero.
        capacity = PABB2_ConnectionSender_BUFFER_SIZE;
        offset = 0;
        buffer_tail = 0;
        self->buffer_head = 0;
        self->buffer_tail = 0;
    }else{
        offset = buffer_tail;

        if (buffer_head >= buffer_tail){
            capacity = buffer_head - buffer_tail;
        }else{
            capacity = PABB2_ConnectionSender_BUFFER_SIZE - buffer_tail;
            if (capacity < bytes){
                offset = 0;
                capacity = buffer_head;
                buffer_tail = 0;
            }
        }
    }

    //  There is no contiguous space large enough.
    if (capacity < bytes){
        return NULL;
    }

    buffer_tail += bytes;
    if (buffer_tail == PABB2_ConnectionSender_BUFFER_SIZE){
        buffer_tail = 0;
    }
    self->buffer_tail = buffer_tail;

    self->offsets[self->slot_tail & PABB2_ConnectionSender_SLOTS_MASK] = ~offset;

    pabb2_PacketHeader* ret = (pabb2_PacketHeader*)(self->buffer + offset);

    ret->seqnum = self->slot_tail++;
    ret->packet_bytes = bytes;

    return ret;
}

void pabb2_PacketSender_commit_packet(pabb2_PacketSender* self, pabb2_PacketHeader* packet){
    packet->magic_number = PABB2_CONNECTION_PACKET_MAGIC_NUMBER;

    pabb_crc32_write_to_message(packet, packet->packet_bytes);

    //  In order to save memory, we repurpose the magic number as a timer to
    //  track when it should be retransmitted. However, it will be set back to
    //  the magic number when sent.
    packet->magic_number = self->retransmit_seqnum;
}

size_t pabb2_PacketSender_send_stream(
    pabb2_PacketSender* self,
    const void* data, size_t bytes
){
    size_t sent = 0;

    while (bytes > 0){
        //  No slots available.
        uint8_t slots_used = self->slot_tail - self->slot_head;
        if (slots_used == PABB2_ConnectionSender_SLOTS){
            break;
        }

        const size_t OVERHEAD = sizeof(pabb2_PacketHeaderData) + sizeof(uint32_t);

        size_t buffer_head = self->buffer_head;
        size_t buffer_tail = self->buffer_tail;

        size_t capacity;
        size_t offset;

        if (slots_used == 0){
            //  We're empty. Rewind both to zero.
            capacity = PABB2_ConnectionSender_BUFFER_SIZE;
            offset = 0;
            buffer_tail = 0;
            self->buffer_head = 0;
            self->buffer_tail = 0;
        }else{
            offset = buffer_tail;

            if (buffer_head >= buffer_tail){
                capacity = buffer_head - buffer_tail;
            }else{
                capacity = PABB2_ConnectionSender_BUFFER_SIZE - buffer_tail;
                if (capacity <= OVERHEAD){
                    offset = 0;
                    capacity = buffer_head;
                    buffer_tail = 0;
                }
            }
        }

        //  There is no contiguous space large enough.
        if (capacity <= OVERHEAD){
            break;
        }

        //  256 will overflow to 0 which is explicitly supported.
        if (capacity > 256){
            capacity = 256;
        }

        size_t current = bytes;
        if (current > capacity - OVERHEAD){
            current = capacity - OVERHEAD;
        }

        size_t packet_bytes = current + OVERHEAD;

        buffer_tail += packet_bytes;
        if (buffer_tail == PABB2_ConnectionSender_BUFFER_SIZE){
            buffer_tail = 0;
        }
        self->buffer_tail = buffer_tail;

        self->offsets[self->slot_tail & PABB2_ConnectionSender_SLOTS_MASK] = ~offset;

        //  Build the packet header.
        pabb2_PacketHeaderData* packet = (pabb2_PacketHeaderData*)(self->buffer + offset);
        packet->magic_number = PABB2_CONNECTION_PACKET_MAGIC_NUMBER;
        packet->seqnum = self->slot_tail++;
        packet->packet_bytes = (uint8_t)packet_bytes;  //  256 overflows to 0
        packet->opcode = PABB2_CONNECTION_PACKET_OPCODE_STREAM_DATA;
        memcpy(&packet->stream_offset, &self->stream_offset, sizeof(uint16_t));   //  May be misaligned.

        //  Copy stream data.
        memcpy(packet + 1, data, current);

        //  Build CRC
        pabb_crc32_write_to_message(packet, packet_bytes);

        //  Send
        self->sender(self->sender_context, packet, packet_bytes);

        //  Set the retransmit timer.
        packet->magic_number = self->retransmit_seqnum;

        sent += current;
        data = (const char*)data + current;
        bytes -= current;
    }

    return sent;
}

bool pabb2_PacketSender_iterate_retransmits(pabb2_PacketSender* self){
    uint8_t head = self->slot_head;
    uint8_t tail = self->slot_tail;

    //  Queue is empty.
    if (head == tail){
        return false;
    }

    uint8_t seqnum = self->retransmit_seqnum;

    while (head != tail){
        size_t offset = ~self->offsets[head & PABB2_ConnectionSender_SLOTS_MASK];
        pabb2_PacketHeader* packet = (pabb2_PacketHeader*)(self->buffer + offset);

        //  Retransmit if it hasn't been acked already and is old enough.
        if (packet->opcode != PABB2_CONNECTION_PACKET_OPCODE_INVALID &&
            seqnum - packet->magic_number >= PABB2_ConnectionSender_RETRANSMIT_COUNTER
        ){
            packet->magic_number = PABB2_CONNECTION_PACKET_MAGIC_NUMBER;
            uint8_t packet_bytes = packet->packet_bytes;
            self->sender(
                self->sender_context,
                packet,
                packet_bytes == 0 ? (size_t)256 : (size_t)packet_bytes
            );
            packet->magic_number = seqnum;
            return true;
        }

        head++;
    }

    //  Increment counter only if we did nothing.
    self->retransmit_seqnum = seqnum + 1;

    return false;
}







