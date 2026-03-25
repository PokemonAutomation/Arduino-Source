/*  PABotBase2 Stream Coalescer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "PABotBase2_StreamCoalescer.h"

//#include <stdio.h>
//#include "PABotBase2_ConnectionDebug.h"

//#include <iostream>
//using std::cout;
//using std::endl;

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wtautological-constant-out-of-range-compare"
#endif

namespace PokemonAutomation{
namespace PABotBase2{



void StreamCoalescer::reset(){
    m_slot_head = 0;
    m_slot_tail = 0;
    m_stream_head = 0;
    m_stream_tail = 0;
    memset(m_lengths, 0, sizeof(m_lengths));

}


void StreamCoalescer::write_buffer(
    const void* data,
    uint16_t stream_offset, uint8_t bytes
){
    if (bytes == 0){
        return;
    }

    uint16_t stream_offset_s = stream_offset;
    uint16_t stream_offset_e = stream_offset + bytes;

    stream_offset_s &= BUFFER_MASK;
    stream_offset_e &= BUFFER_MASK;

    if (stream_offset_s < stream_offset_e){
        memcpy(m_buffer + stream_offset_s, data, bytes);
    }else{
        //  Wrap around
        uint8_t block = (uint8_t)(BUFFER_SIZE - stream_offset_s);
        memcpy(m_buffer + stream_offset_s, data, block);
        memcpy(m_buffer, (const uint8_t*)data + block, bytes - block);
    }
}
void StreamCoalescer::read_buffer(
    void* data,
    uint16_t stream_offset, uint8_t bytes
){
    if (bytes == 0){
        return;
    }

    uint16_t stream_offset_s = stream_offset;
    uint16_t stream_offset_e = stream_offset + bytes;

    stream_offset_s &= BUFFER_MASK;
    stream_offset_e &= BUFFER_MASK;

    if (stream_offset_s < stream_offset_e){
        memcpy(data, m_buffer + stream_offset_s, bytes);
    }else{
        //  Wrap around
        uint8_t block = (uint8_t)(BUFFER_SIZE - stream_offset_s);
        memcpy(data, m_buffer + stream_offset_s, block);
        memcpy((uint8_t*)data + block, m_buffer, bytes - block);
    }
}
void StreamCoalescer::pop_leading_finished(){
    uint8_t slot_head = m_slot_head;
    uint8_t slot_tail = m_slot_tail;

//    printf("pabb2_StreamCoalescer_pop_leading_finished()\n");

    while (slot_head != slot_tail){
        uint8_t* slot = &m_lengths[slot_head & SLOTS_MASK];
//        printf("slot[%d] = %d\n", slot_head, *slot);
        if (*slot != 0xff){
            break;
        }
        *slot = 0;
        slot_head++;
    }

    m_slot_head = slot_head;
}


uint16_t StreamCoalescer::free_bytes() const{
    if (m_slot_head == m_slot_tail){
        return BUFFER_SIZE;
    }
    return (m_stream_tail - m_stream_head) & BUFFER_MASK;
}

void StreamCoalescer::push_packet(uint8_t seqnum){
    uint8_t slot_head = m_slot_head;

//    printf("enter ---------------------\n");
//    pabb2_StreamCoalescer_print(self, false);

//    printf("pabb2_StreamCoalescer_push_packet(%p): seqnum = %d, slot_head = %d\n", self, seqnum, slot_head);

    //  Either before (old retransmit) or too far in future.
    if ((uint8_t)(seqnum - slot_head) >= SLOTS){
//        printf("Device: Packet is out of range.\n");
        return;
    }

    //  Extend the tail if needed.
    uint8_t slot_tail = m_slot_tail;
    if ((uint8_t)(seqnum - slot_tail) < SLOTS){
        slot_tail = seqnum + 1;
        m_slot_tail = slot_tail;
    }

    //  Mark slot as done.
    m_lengths[seqnum & SLOTS_MASK] = 0xff;

    //  Pop all finished slots at the head of the queue.
    pop_leading_finished();


//    printf("exit ---------------------\n");
//    pabb2_StreamCoalescer_print(self, false);
}

bool StreamCoalescer::push_stream(const PacketHeaderData* packet){
    pop_leading_finished();

    uint8_t stream_size = packet->packet_bytes - sizeof(PacketHeaderData) - sizeof(uint32_t);

    //  Zero does not fall through cleanly. So handle it here.
    if (stream_size == 0){
//        printf("Device: stream_size == 0\n");
        push_packet(packet->seqnum);
        return true;
    }

    //  Data is larger than the entire buffer.
    if (stream_size > BUFFER_SIZE){
//        printf("Device: stream_size > BUFFER_SIZE\n");
        return false;
    }

    uint8_t seqnum = packet->seqnum;
    uint8_t slot_head = m_slot_head;

//    {
//        std::lock_guard<std::mutex> lg(print_lock);
//        cout << (int)packet->seqnum << " : " << packet->stream_offset << endl;
//    }

    //  Either before (old retransmit) or too far in future.
    {
        uint8_t diff = seqnum - slot_head;
//        printf("seqnum = %d, slot_head = %d\n", seqnum, slot_head);
        if (diff >= SLOTS){
//            printf("seqnum = %d, slot_head = %d\n", seqnum, slot_head);
//            printf("Device: In the past or too far ahead.\n");
            //  Negative means we're in the past and we can just ack.
            return diff & 0x80;
        }
    }

    uint16_t stream_offset_s = packet->stream_offset;
    uint16_t stream_offset_e = stream_offset_s + stream_size;

//    size_t stream_head = m_stream_head;

    //  Too far ahead that it's beyond our window.
    if ((uint16_t)(stream_offset_e - m_stream_head) > BUFFER_SIZE){
//        printf("Device: To far in future.\n");
        return false;
    }

    //  Extend the tail if needed.
    uint8_t slot_tail = m_slot_tail;
    if ((uint8_t)(seqnum - slot_tail) < SLOTS){
        slot_tail = seqnum + 1;
        m_slot_tail = slot_tail;
    }
    uint16_t stream_tail = m_stream_tail;
    if ((uint16_t)(stream_offset_e - stream_tail) < BUFFER_SIZE){
        stream_tail = stream_offset_e;
        m_stream_tail = stream_tail;
    }

    uint8_t index = seqnum & SLOTS_MASK;
    m_offsets[index] = stream_offset_s;
    m_lengths[index] = stream_size;

//    cout << "index = " << (int)index << ", stream_offset_s = " << stream_offset_s << ", bytes = " << (int)stream_size << endl;
    write_buffer(packet + 1, stream_offset_s, stream_size);

    return true;
}

size_t StreamCoalescer::read(void* data, size_t max_bytes){
    size_t read = 0;

    while (max_bytes > 0){
        uint8_t slot_head = m_slot_head;
        uint8_t slot_tail = m_slot_tail;

        //  Queue is empty.
        if (slot_head == slot_tail){
            break;
        }

        uint8_t index = slot_head & SLOTS_MASK;
        uint8_t length = m_lengths[index];

        //  Front of the queue is a hole. We don't have the data yet.
        if (length == 0){
            break;
        }

        uint16_t valid_to = m_offsets[index] + length;


        //  Calculate the read boundaries.
        uint16_t stream_head;
        {
            stream_head = m_stream_head;

            //  Try to consume the entire packet.
            uint16_t try_read = valid_to - stream_head;
            if (try_read > max_bytes){
                //  Nope, we're capped by "max_bytes".
                length = (uint8_t)max_bytes;
            }else{
                //  We're consuming the entire packet. Remove from queue.
                length = (uint8_t)try_read;
                m_lengths[index] = 0xff;
                pop_leading_finished();
            }

            if (length == 0){
                return read;
            }

//            cout << "m_stream_head = " << m_stream_head << " -> " << m_stream_head + length << endl;
            m_stream_head += length;
        }

        read_buffer(data, stream_head, length);

        read += length;
        data = (char*)data + length;
        max_bytes -= length;
    }

    return read;
}

















}
}
