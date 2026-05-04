/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/Compiler.h"
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
    reset(0);
}
PacketSender::PacketSender(
    UnreliableStreamSender& connection,
    uint8_t max_packet_size,
    SessionId session_id
)
    : m_connection(connection)
    , m_max_packet_size(max_packet_size)
{
    reset(session_id);
}
void PacketSender::reset(const SessionId& session_id){
    memcpy(&m_session_id, &session_id, sizeof(SessionId));
    m_slot_head = 0;
    m_slot_tail = 0;
    m_slot_tail_uncommitted = 0;
    m_retransmit_seqnum = 0;
    m_stream_corrupted = false;
    m_stream_offset = 0;
    m_stream_offset_uncommitted = 0;
    m_buffer_head = 0;
    m_buffer_tail = 0;
    m_buffer_tail_uncommitted = 0;
}




PA_NO_INLINE void PacketSender::send_oob_packet_empty(uint8_t seqnum, uint8_t opcode){
    struct{
        PacketHeader header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    pabb_crc32_write_to_message(m_session_id, &packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet));
}
PA_NO_INLINE void PacketSender::send_oob_packet_u8(uint8_t seqnum, uint8_t opcode, uint8_t data){
    struct{
        PacketHeader_u8 header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    packet.header.data = data;
    pabb_crc32_write_to_message(m_session_id, &packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet));
}
PA_NO_INLINE void PacketSender::send_oob_packet_u16(uint8_t seqnum, uint8_t opcode, const uint16_t& data){
    struct{
        PacketHeader_u16 header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    memcpy(&packet.header.data, &data, sizeof(uint16_t));
    pabb_crc32_write_to_message(m_session_id, &packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet));
}
PA_NO_INLINE void PacketSender::send_oob_packet_u32(uint8_t seqnum, uint8_t opcode, const uint32_t& data){
    struct{
        PacketHeader_u32 header;
        uint8_t crc32[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    memcpy(&packet.header.data, &data, sizeof(uint32_t));
    pabb_crc32_write_to_message(m_session_id, &packet, sizeof(packet));
    m_connection.unreliable_send(&packet, sizeof(packet));
}
PA_NO_INLINE void PacketSender::send_oob_packet_data(
    uint8_t seqnum, uint8_t opcode,
    uint8_t bytes, const void* data
){
    PacketHeader header;
    header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    header.seqnum = seqnum;
    header.packet_bytes = sizeof(PacketHeader) + sizeof(uint32_t) + bytes;
    header.opcode = opcode;
    uint32_t crc = m_session_id;
    pabb_crc32_buffer(&crc, &header, sizeof(PacketHeader));
    pabb_crc32_buffer(&crc, data, bytes);
    m_connection.unreliable_send(&header, sizeof(header));
    m_connection.unreliable_send(data, bytes);
    m_connection.unreliable_send(&crc, sizeof(uint32_t));
}
PA_NO_INLINE void PacketSender::send_oob_packet_u32_data(
    uint8_t seqnum, uint8_t opcode,
    const uint32_t& u32,
    uint8_t bytes, const void* data
){
    PacketHeader_u32 header;
    header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    header.seqnum = seqnum;
    header.packet_bytes = sizeof(PacketHeader_u32) + sizeof(uint32_t) + bytes;
    header.opcode = opcode;
    memcpy(&header.data, &u32, sizeof(uint32_t));
    uint32_t crc = m_session_id;
    pabb_crc32_buffer(&crc, &header, sizeof(PacketHeader_u32));
    pabb_crc32_buffer(&crc, data, bytes);
    m_connection.unreliable_send(&header, sizeof(header));
    m_connection.unreliable_send(data, bytes);
    m_connection.unreliable_send(&crc, sizeof(uint32_t));
}




bool PacketSender::remove(uint8_t seqnum){
//    {
//        std::lock_guard<std::mutex> lg(print_lock);
//        cout << "PacketSender::remove(" << this << "): " << (int)seqnum << endl;
//    }

    //  Too far in the future.
    if ((uint8_t)(seqnum - m_slot_head) >= SLOTS){
        return false;
    }

    //  Too far in the past.
    if ((uint8_t)(m_slot_tail - seqnum) > SLOTS){
        return false;
    }

    //  Mark the slot has invalid.
    {
        size_t offset = m_offsets[seqnum & SLOTS_MASK];
        PacketHeader* packet = (PacketHeader*)(m_buffer + offset);
        packet->opcode = PABB2_CONNECTION_OPCODE_INVALID;
    }

    //  Not the front.
    if (seqnum != m_slot_head){
        return true;
    }

    //  Clear out all acked requests from the front of the queue.
    while (true){
        seqnum++;
        m_slot_head = seqnum;

        //  Queue is empty.
        if (seqnum == m_slot_tail){
            m_buffer_head = 0;
            m_buffer_tail = 0;
            return true;
        }

        size_t offset = m_offsets[seqnum & SLOTS_MASK];
        m_buffer_head = offset;

        PacketHeader* packet = (PacketHeader*)(m_buffer + offset);

        //  Packet hasn't been acked yet. We can't go further.
        if (packet->opcode != PABB2_CONNECTION_OPCODE_INVALID){
            return true;
        }
    }
}

void PacketSender::send_reset(){
    PacketHeader_u32* packet = (PacketHeader_u32*)reserve_packet(
        PABB2_CONNECTION_OPCODE_ASK_RESET,
        sizeof(PacketHeader_u32) - sizeof(PacketHeader)
    );
    if (packet == NULL){
        return;
    }

    memcpy(&packet->data, &m_session_id, sizeof(uint32_t));

    //  We temporarily change the session ID to 0xffffffff
    //  so that the CRC is generated correctly.
    uint32_t tmp = m_session_id;
    m_session_id = 0xffffffff;

    commit_packet(packet);

    m_session_id = tmp;

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
    if (slots_used == SLOTS){
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
        m_buffer_tail_uncommitted = 0;
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

    m_offsets[m_slot_tail & SLOTS_MASK] = offset;

    PacketHeader* ret = (PacketHeader*)(m_buffer + offset);

    ret->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    ret->seqnum = m_slot_tail++;
    ret->packet_bytes = packet_bytes;
    ret->opcode = opcode;
    m_slot_tail_uncommitted = m_slot_tail;

    return ret;
}
void PacketSender::commit_packet(PacketHeader* packet){
    pabb_crc32_write_to_message(m_session_id, packet, packet->packet_bytes);

    m_connection.unreliable_send(packet, packet->packet_bytes);

    //  In order to save memory, we repurpose the magic number as a timer to
    //  track when it should be retransmitted. However, it will be set back to
    //  the magic number when sent.
    packet->magic_number = m_retransmit_seqnum;
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
        size_t offset = m_offsets[head & SLOTS_MASK];
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

        packet->opcode |= PABB2_CONNECTION_RETRANSMIT_FLAG;
        packet->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
        uint8_t packet_bytes = packet->packet_bytes;
        pabb_crc32_write_to_message(m_session_id, packet, packet_bytes);

#if 0
        printf("Retransmitting: %u\n", packet->seqnum);
        fflush(stdout);
#endif

        m_connection.unreliable_send(
            packet,
            packet_bytes == 0 ? (size_t)256 : (size_t)packet_bytes
        );
        packet->magic_number = seqnum;
        return true;
    }

    //  Increment counter only if we did nothing.
    m_retransmit_seqnum = seqnum + 1;

    return false;
}





bool PacketSender::enqueue_uncommitted_send_stream(const void* data, size_t bytes) noexcept{
    if (m_stream_corrupted){
        return false;
    }

    //  256 will overflow to 0 which is explicitly supported.
    size_t max_packet_size = (uint8_t)(m_max_packet_size - 1);
    max_packet_size++;

    while (bytes > 0){
        //  No slots available.
        uint8_t slots_used = m_slot_tail_uncommitted - m_slot_head;
        if (slots_used == SLOTS){
            break;
        }

        const size_t OVERHEAD = sizeof(PacketHeaderData) + sizeof(uint32_t);


        size_t capacity;
        size_t offset;

        if (slots_used == 0){
//            printf("slots used: %u\n", slots_used);
            //  We're empty. Rewind both to zero.
            capacity = PABB2_PacketSender_BUFFER_SIZE;
            offset = 0;
            m_buffer_tail_uncommitted = 0;
        }else{
            offset = m_buffer_tail_uncommitted;

            if (m_buffer_head >= m_buffer_tail_uncommitted){
                capacity = m_buffer_head - m_buffer_tail_uncommitted;
            }else{
                capacity = PABB2_PacketSender_BUFFER_SIZE - m_buffer_tail_uncommitted;
                if (capacity <= OVERHEAD){
                    offset = 0;
                    capacity = m_buffer_head;
                    m_buffer_tail_uncommitted = 0;
                }
            }
        }

        //  There is no contiguous space large enough.
        if (capacity <= OVERHEAD){
            break;
        }

        if (capacity > max_packet_size){
            capacity = max_packet_size;
        }

        size_t current = bytes;
        if (current > capacity - OVERHEAD){
            current = capacity - OVERHEAD;
        }

        size_t packet_bytes = current + OVERHEAD;

//        printf("buffer_tail: %zu, packet_bytes: %zu\n", buffer_tail, packet_bytes);

        m_buffer_tail_uncommitted += packet_bytes;
        if (m_buffer_tail_uncommitted == PABB2_PacketSender_BUFFER_SIZE){
            m_buffer_tail_uncommitted = 0;
        }
//        printf("self->buffer_tail: %zu\n", self->buffer_tail);

        m_offsets[m_slot_tail_uncommitted & SLOTS_MASK] = offset;

        //  Build the packet header.
        PacketHeaderData* packet = (PacketHeaderData*)(m_buffer + offset);
        packet->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
        packet->seqnum = m_slot_tail_uncommitted++;
        packet->packet_bytes = (uint8_t)packet_bytes;  //  256 overflows to 0
        packet->opcode = PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA;
        memcpy(&packet->stream_offset, &m_stream_offset_uncommitted, sizeof(uint16_t));   //  May be misaligned.

        //  Copy stream data.
        memcpy(packet + 1, data, current);
        m_stream_offset_uncommitted += (uint16_t)current;

        //  Build CRC
        pabb_crc32_write_to_message(m_session_id, packet, packet_bytes);

        data = (const char*)data + current;
        bytes -= current;
    }

    if (bytes == 0){
        return true;
    }
    abort_uncommitted_send_stream();
    return false;
}
void PacketSender::abort_uncommitted_send_stream(){
    m_slot_tail_uncommitted = m_slot_tail;
    m_buffer_tail_uncommitted = m_buffer_tail;
    m_stream_offset_uncommitted = m_stream_offset;
}
void PacketSender::commit_uncommitted_send_stream() noexcept{
    //  Able to send. Commit the sends.
    m_buffer_tail = m_buffer_tail_uncommitted;
    m_stream_offset = m_stream_offset_uncommitted;

    while (m_slot_tail != m_slot_tail_uncommitted){
        size_t offset = m_offsets[m_slot_tail++ & SLOTS_MASK];
        PacketHeader* packet = (PacketHeader*)(m_buffer + offset);

        //  Send
//        cout << "Send: " << (unsigned)packet->seqnum << ", Bytes: " << (unsigned)packet->packet_bytes << endl;
        m_connection.unreliable_send(
            packet,
            packet->packet_bytes == 0 ? 256 : packet->packet_bytes
        );

        //  Set the retransmit timer.
        packet->magic_number = m_retransmit_seqnum;
    }
}














}
}
