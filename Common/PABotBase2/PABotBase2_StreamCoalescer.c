/*  PABotBase2 Stream Coalescer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "PABotBase2_StreamCoalescer.h"

#include <stdio.h>  //  REMOVE
#include "PABotBase2_ConnectionDebug.h" //  REMOVE

void pabb2_StreamCoalescer_init(pabb2_StreamCoalescer* self){
    self->slot_head = 0;
    self->slot_tail = 0;
    self->stream_head = 0;
    self->stream_tail = 0;
    self->stream_reset = false;
    memset(self->lengths, 0, sizeof(self->lengths));
}


void pabb2_StreamCoalescer_write_buffer(
    pabb2_StreamCoalescer* self,
    const void* data,
    uint16_t stream_offset, uint8_t bytes
){
    uint16_t stream_offset_s = stream_offset;
    uint16_t stream_offset_e = stream_offset + bytes;

    stream_offset_s &= PABB2_StreamCoalescer_BUFFER_MASK;
    stream_offset_e &= PABB2_StreamCoalescer_BUFFER_MASK;

    if (stream_offset_s < stream_offset_e){
        memcpy(self->buffer + stream_offset_s, data, bytes);
    }else{
        //  Wrap around
        uint8_t block = (uint8_t)(PABB2_StreamCoalescer_BUFFER_SIZE - stream_offset_s);
        memcpy(self->buffer + stream_offset_s, data, block);
        memcpy(self->buffer, (const uint8_t*)data + block, bytes - block);
    }
}
void pabb2_StreamCoalescer_read_buffer(
    const pabb2_StreamCoalescer* self,
    void* data,
    uint16_t stream_offset, uint8_t bytes
){
    uint16_t stream_offset_s = stream_offset;
    uint16_t stream_offset_e = stream_offset + bytes;

    stream_offset_s &= PABB2_StreamCoalescer_BUFFER_MASK;
    stream_offset_e &= PABB2_StreamCoalescer_BUFFER_MASK;

    if (stream_offset_s < stream_offset_e){
        memcpy(data, self->buffer + stream_offset_s, bytes);
    }else{
        //  Wrap around
        uint8_t block = (uint8_t)(PABB2_StreamCoalescer_BUFFER_SIZE - stream_offset_s);
        memcpy(data, self->buffer + stream_offset_s, block);
        memcpy((uint8_t*)data + block, self->buffer, bytes - block);
    }
}
void pabb2_StreamCoalescer_pop_leading_finished(pabb2_StreamCoalescer* self){
    uint8_t slot_head = self->slot_head;
    uint8_t slot_tail = self->slot_tail;

    while (slot_head != slot_tail){
        uint8_t* slot = &self->lengths[slot_head & PABB2_StreamCoalescer_SLOTS_MASK];
        if (*slot != 0xff){
            break;
        }
        *slot = 0;
        slot_head++;
    }

    self->slot_head = slot_head;
}


uint16_t pabb2_StreamCoalescer_bytes_available(pabb2_StreamCoalescer* self){
    if (self->slot_head == self->slot_tail){
        return PABB2_StreamCoalescer_BUFFER_SIZE;
    }
    return (self->stream_tail - self->stream_head) & PABB2_StreamCoalescer_BUFFER_MASK;
}

void pabb2_StreamCoalescer_push_packet(pabb2_StreamCoalescer* self, uint8_t seqnum){
    uint8_t slot_head = self->slot_head;

//    printf("enter ---------------------\n");
//    pabb2_StreamCoalescer_print(self, false);

    //  Either before (old retransmit) or too far in future.
    if ((uint8_t)(seqnum - slot_head) >= PABB2_StreamCoalescer_SLOTS){
        return;
    }

    //  Extend the tail if needed.
    uint8_t slot_tail = self->slot_tail;
    if ((uint8_t)(seqnum - slot_tail) < PABB2_StreamCoalescer_SLOTS){
        slot_tail = seqnum + 1;
        self->slot_tail = slot_tail;
    }

    //  Mark slot as done.
    self->lengths[seqnum & PABB2_StreamCoalescer_SLOTS_MASK] = 0xff;

    //  Pop all finished slots at the head of the queue.
    pabb2_StreamCoalescer_pop_leading_finished(self);


//    printf("exit ---------------------\n");
//    pabb2_StreamCoalescer_print(self, false);
}

bool pabb2_StreamCoalescer_push_stream(pabb2_StreamCoalescer* self, const pabb2_PacketHeaderData* packet){
    uint8_t stream_size = packet->packet_bytes - sizeof(pabb2_PacketHeaderData) - sizeof(uint32_t);

    //  Zero does not fall through cleanly. So handle it here.
    if (stream_size == 0){
        pabb2_StreamCoalescer_push_packet(self, packet->seqnum);
        return true;
    }

    //  Data is larger than the entire buffer.
    if (stream_size > PABB2_StreamCoalescer_BUFFER_SIZE){
        return false;
    }

    uint8_t seqnum = packet->seqnum;
    uint8_t slot_head = self->slot_head;

    //  Either before (old retransmit) or too far in future.
    {
        uint8_t diff = seqnum - slot_head;
        if (diff >= PABB2_StreamCoalescer_SLOTS){
            //  Negative means we're in the past and we can just ack.
            return diff & 0x80;
        }
    }

    uint16_t stream_offset_s = packet->stream_offset;
    uint16_t stream_offset_e = stream_offset_s + stream_size;

//    size_t stream_head = self->stream_head;

    //  Too far ahead that it's beyond our window.
    if (stream_offset_e - self->stream_head > PABB2_StreamCoalescer_BUFFER_SIZE){
        return false;
    }

    //  Extend the tail if needed.
    uint8_t slot_tail = self->slot_tail;
    if ((uint8_t)(seqnum - slot_tail) < PABB2_StreamCoalescer_SLOTS){
        slot_tail = seqnum + 1;
        self->slot_tail = slot_tail;
    }
    uint16_t stream_tail = self->stream_tail;
    if ((uint16_t)(stream_offset_e - stream_tail) < PABB2_StreamCoalescer_BUFFER_SIZE){
        stream_tail = stream_offset_e;
        self->stream_tail = stream_tail;
    }

    uint8_t index = seqnum & PABB2_StreamCoalescer_SLOTS_MASK;
    self->offsets[index] = stream_offset_s;
    self->lengths[index] = stream_size;

    pabb2_StreamCoalescer_write_buffer(self, packet + 1, stream_offset_s, stream_size);

    return true;
}

size_t pabb2_StreamCoalescer_read(pabb2_StreamCoalescer* self, void* data, size_t max_bytes){
    if (self->stream_reset){
        self->stream_reset = false;
        return (size_t)-1;
    }

    size_t read = 0;

    while (max_bytes > 0){
        uint8_t slot_head = self->slot_head;
        uint8_t slot_tail = self->slot_tail;

        //  Queue is empty.
        if (slot_head == slot_tail){
            break;
        }

        uint8_t index = slot_head & PABB2_StreamCoalescer_SLOTS_MASK;
        uint8_t length = self->lengths[index];

        //  Front of the queue is a hole. We don't have the data yet.
        if (length == 0){
            break;
        }

        uint16_t valid_to = self->offsets[index] + length;


        //  Calculate the read boundaries.
        uint16_t stream_head;
        {
            stream_head = self->stream_head;

            //  Try to consume the entire packet.
            uint16_t try_read = valid_to - stream_head;
            if (try_read > max_bytes){
                //  Nope, we're capped by "max_bytes".
                length = (uint8_t)max_bytes;
            }else{
                //  We're consuming the entire packet. Remove from queue.
                length = (uint8_t)try_read;
                self->lengths[index] = 0xff;
                pabb2_StreamCoalescer_pop_leading_finished(self);
            }

            self->stream_head += length;
        }

        pabb2_StreamCoalescer_read_buffer(self, data, stream_head, length);

        read += length;
        data = (char*)data + length;
        max_bytes -= length;
    }

    return read;
}


















