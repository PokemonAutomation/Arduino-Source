/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_PacketSender.h"

//#include <stdio.h>
//#include "PABotBase2_ConnectionDebug.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{

PacketSender::PacketSender(
    UnreliableStreamSender& connection,
    uint8_t max_packet_size
)
    : m_connection(connection)
    , m_max_packet_size(max_packet_size)
{
    reset();
}
void PacketSender::reset(){
    m_slot_head = 0;
    m_slot_tail = 0;
    m_retransmit_seqnum = 0;
    m_stream_offset = 0;
    m_buffer_head = 0;
    m_buffer_tail = 0;
    memset(m_offsets, 0, sizeof(m_offsets));
}

bool PacketSender::remove(uint8_t seqnum){
//    {
//        std::lock_guard<std::mutex> lg(print_lock);
//        cout << "PacketSender::remove(" << this << "): " << (int)seqnum << endl;
//    }
    {
        size_t offset = m_offsets[seqnum & SLOTS_MASK];
        if (offset == 0){
            return false;
        }
        offset = ~offset;
        PacketHeader* packet = (PacketHeader*)(m_buffer + offset);
        if (packet->seqnum != seqnum){
            return false;
        }
        packet->opcode = PABB2_CONNECTION_OPCODE_INVALID;
    }

    //  Not the front.
    if (seqnum != m_slot_head){
        return true;
    }

    //  Clear out all acked requests from the front of the queue.
    while (true){
        m_offsets[seqnum & SLOTS_MASK] = 0;
        seqnum++;
        m_slot_head = seqnum;

        //  Queue is empty.
        if (seqnum == m_slot_tail){
            m_buffer_head = 0;
            m_buffer_tail = 0;
            return true;
        }

        size_t offset = ~m_offsets[seqnum & SLOTS_MASK];
        m_buffer_head = offset;

        PacketHeader* packet = (PacketHeader*)(m_buffer + offset);

        //  Packet hasn't been acked yet. We can't go further.
        if (packet->opcode != PABB2_CONNECTION_OPCODE_INVALID){
            return true;
        }
    }
}

bool PacketSender::send_packet(
    uint8_t opcode, uint8_t extra_bytes, const void* extra_data
){
    PacketHeader* packet = reserve_packet(opcode, extra_bytes);
    if (packet == NULL){
        return false;
    }
    memcpy(packet + 1, extra_data, extra_bytes);
    commit_packet(packet);
    return true;
}

PacketHeader* PacketSender::reserve_packet(
    uint8_t opcode, uint8_t extra_bytes
){
    //  No slots available.
    uint8_t slots_used = m_slot_tail - m_slot_head;
    if (slots_used == PABB2_PacketSender_SLOTS){
        return NULL;
    }

    uint8_t packet_bytes = extra_bytes + sizeof(PacketHeader) + sizeof(uint32_t);

    size_t buffer_head = m_buffer_head;
    size_t buffer_tail = m_buffer_tail;

    size_t capacity;
    size_t offset;

    if (slots_used == 0){
        //  We're empty. Rewind both to zero.
        capacity = PABB2_PacketSender_BUFFER_SIZE;
        offset = 0;
        buffer_tail = 0;
        m_buffer_head = 0;
        m_buffer_tail = 0;
    }else{
        offset = buffer_tail;

        if (buffer_head >= buffer_tail){
            capacity = buffer_head - buffer_tail;
        }else{
            capacity = PABB2_PacketSender_BUFFER_SIZE - buffer_tail;
            if (capacity < packet_bytes){
                offset = 0;
                capacity = buffer_head;
                buffer_tail = 0;
            }
        }
    }

    //  There is no contiguous space large enough.
    if (capacity < packet_bytes){
        return NULL;
    }

    buffer_tail += packet_bytes;
    if (buffer_tail == PABB2_PacketSender_BUFFER_SIZE){
        buffer_tail = 0;
    }
    m_buffer_tail = buffer_tail;

    m_offsets[m_slot_tail & SLOTS_MASK] = ~offset;

    PacketHeader* ret = (PacketHeader*)(m_buffer + offset);

    ret->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    ret->seqnum = m_slot_tail++;
    ret->packet_bytes = packet_bytes;
    ret->opcode = opcode;

    return ret;
}

void PacketSender::commit_packet(PacketHeader* packet){
    pabb_crc32_write_to_message(packet, packet->packet_bytes);

    m_connection.unreliable_send(
        packet, packet->packet_bytes,
        false
    );

    //  In order to save memory, we repurpose the magic number as a timer to
    //  track when it should be retransmitted. However, it will be set back to
    //  the magic number when sent.
    packet->magic_number = m_retransmit_seqnum;
}

size_t PacketSender::send_stream(const void* data, size_t bytes){
    size_t sent = 0;

    while (bytes > 0){
        //  No slots available.
        uint8_t slots_used = m_slot_tail - m_slot_head;
        if (slots_used == PABB2_PacketSender_SLOTS){
            break;
        }

        const size_t OVERHEAD = sizeof(PacketHeaderData) + sizeof(uint32_t);

        size_t buffer_head = m_buffer_head;
        size_t buffer_tail = m_buffer_tail;

        size_t capacity;
        size_t offset;

        if (slots_used == 0){
//            printf("slots used: %u\n", slots_used);
            //  We're empty. Rewind both to zero.
            capacity = PABB2_PacketSender_BUFFER_SIZE;
            offset = 0;
            buffer_tail = 0;
            m_buffer_head = 0;
            m_buffer_tail = 0;
        }else{
            offset = buffer_tail;

            if (buffer_head >= buffer_tail){
                capacity = buffer_head - buffer_tail;
            }else{
                capacity = PABB2_PacketSender_BUFFER_SIZE - buffer_tail;
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
        size_t max_packet_size = (uint8_t)(m_max_packet_size - 1);
        max_packet_size++;
        if (capacity > max_packet_size){
            capacity = max_packet_size;
        }

        size_t current = bytes;
        if (current > capacity - OVERHEAD){
            current = capacity - OVERHEAD;
        }

        size_t packet_bytes = current + OVERHEAD;

//        printf("buffer_tail: %zu, packet_bytes: %zu\n", buffer_tail, packet_bytes);

        buffer_tail += packet_bytes;
        if (buffer_tail == PABB2_PacketSender_BUFFER_SIZE){
            buffer_tail = 0;
        }
        m_buffer_tail = buffer_tail;
//        printf("self->buffer_tail: %zu\n", self->buffer_tail);

        m_offsets[m_slot_tail & SLOTS_MASK] = ~offset;

        //  Build the packet header.
        PacketHeaderData* packet = (PacketHeaderData*)(m_buffer + offset);
        packet->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
        packet->seqnum = m_slot_tail++;
        packet->packet_bytes = (uint8_t)packet_bytes;  //  256 overflows to 0
        packet->opcode = PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA;
        memcpy(&packet->stream_offset, &m_stream_offset, sizeof(uint16_t));   //  May be misaligned.

        //  Copy stream data.
        memcpy(packet + 1, data, current);
        m_stream_offset += (uint16_t)current;

        //  Build CRC
        pabb_crc32_write_to_message(packet, packet_bytes);

//        cout << "Send: " << (int)packet->seqnum << ", Offset: " << packet->stream_offset << endl;

        //  Send
        m_connection.unreliable_send(
            packet, packet_bytes,
            false
        );

        //  Set the retransmit timer.
        packet->magic_number = m_retransmit_seqnum;

        sent += current;
        data = (const char*)data + current;
        bytes -= current;
    }

    return sent;
}

bool PacketSender::iterate_retransmits(){
    uint8_t head = m_slot_head;
    uint8_t tail = m_slot_tail;

    //  Queue is empty.
    if (head == tail){
        return false;
    }

    uint8_t seqnum = m_retransmit_seqnum;

    while (head != tail){
        size_t offset = ~m_offsets[head & SLOTS_MASK];
        PacketHeader* packet = (PacketHeader*)(m_buffer + offset);

        //  Already acked.
        if (packet->opcode == PABB2_CONNECTION_OPCODE_INVALID){
#if 0
            printf("Already Acked\n");
            fflush(stdout);
#endif
            head++;
            continue;
        }

        //  Not old enough.
        if ((uint8_t)(seqnum - packet->magic_number) < PABB2_PacketSender_RETRANSMIT_COUNTER){
#if 0
            printf("Not old enough: seqnum = %d, packet = %d\n", seqnum, packet->magic_number);
            fflush(stdout);
#endif
            head++;
            continue;
        }

        packet->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
        uint8_t packet_bytes = packet->packet_bytes;

#if 0
        printf("Retransmitting: %u\n", packet->seqnum);
        fflush(stdout);
#endif

        m_connection.unreliable_send(
            packet,
            packet_bytes == 0 ? (size_t)256 : (size_t)packet_bytes,
            true
        );
        packet->magic_number = seqnum;
        return true;
    }

    //  Increment counter only if we did nothing.
    m_retransmit_seqnum = seqnum + 1;

    return false;
}




void PacketSender::send_oob_packet_empty(uint8_t seqnum, uint8_t opcode){
    struct{
        PacketHeader header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    pabb_crc32_write_to_message(&packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet), false);
}
void PacketSender::send_oob_packet_u8(uint8_t seqnum, uint8_t opcode, uint8_t data){
    struct{
        PacketHeader_Ack_u8 header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    packet.header.data = data;
    pabb_crc32_write_to_message(&packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet), false);
}
void PacketSender::send_oob_packet_u16(uint8_t seqnum, uint8_t opcode, uint16_t data){
    struct{
        PacketHeader_Ack_u16 header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    packet.header.data = data;
    pabb_crc32_write_to_message(&packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet), false);
}
void PacketSender::send_oob_packet_u32(uint8_t seqnum, uint8_t opcode, uint32_t data){
    struct{
        PacketHeader_Ack_u32 header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    packet.header.data = data;
    pabb_crc32_write_to_message(&packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet), false);
}




}
}
