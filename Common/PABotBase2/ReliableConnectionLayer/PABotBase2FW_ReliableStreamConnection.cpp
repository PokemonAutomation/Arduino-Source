/*  PABotBase2 Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include <algorithm>
#include "PABotBase2_ConnectionDebug.h"
#include "PABotBase2FW_ReliableStreamConnection.h"

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


ReliableStreamConnectionFW::ReliableStreamConnectionFW(UnreliableStreamConnectionPolling& unreliable_connection)
    : m_unreliable_connection(unreliable_connection)
    , m_reliable_sender(unreliable_connection, (uint8_t)(PABB2_MAX_INCOMING_PACKET_SIZE % 256))
    , m_last_retransmit(pabb_current_time())
{}



void ReliableStreamConnectionFW::reliable_send(const void* data, size_t bytes){
    if (!m_stream_ready){
        return;
    }
    const char* ptr = (const char*)data;
    while (bytes > 0){
        size_t sent = m_reliable_sender.send_stream(ptr, bytes);

        if (sent == 0){
            m_reliable_sender.declare_stream_corrupted();
            m_reliable_sender.send_oob_packet_empty(0, PABB2_CONNECTION_OPCODE_INFO_STREAM_DEAD);
//            printf("Stream buffer is full.\n");
            return;
        }

        ptr += sent;
        bytes -= sent;
    }
}


void ReliableStreamConnectionFW::send_oob_info_binary(const void* data, uint8_t bytes){
    const size_t MAX_LENGTH = 256 - sizeof(PacketHeader) - sizeof(uint32_t);
    m_reliable_sender.send_oob_packet_data(
        0, PABB2_CONNECTION_OPCODE_INFO_BINARY,
        (uint8_t)std::min<uint8_t>(bytes, MAX_LENGTH), data
    );
}
void ReliableStreamConnectionFW::send_oob_info_str(const char* str){
    const size_t MAX_LENGTH = 256 - sizeof(PacketHeader) - sizeof(uint32_t);
    size_t len = strlen(str);
    m_reliable_sender.send_oob_packet_data(
        0, PABB2_CONNECTION_OPCODE_INFO_STR,
        (uint8_t)std::min(len, MAX_LENGTH), str
    );
}
void ReliableStreamConnectionFW::send_oob_info_label_i32(uint8_t opcode, const char* str, uint32_t data){
    const size_t MAX_LENGTH = 256 - sizeof(PacketHeader_u32) - sizeof(uint32_t);
    size_t len = strlen(str);
    m_reliable_sender.send_oob_packet_u32_data(
        0, opcode,
        data,
        (uint8_t)std::min(len, MAX_LENGTH), str
    );
}



void ReliableStreamConnectionFW::wait_for_event(uint16_t milliseconds){
    //  If we have unacked sends, we cap the wait time since those may need to
    //  be retransmitted.
    if (m_reliable_sender.slots_used() != 0 && milliseconds > PABB2_ReliableConnectionFW_POLL_MS){
        milliseconds = PABB2_ReliableConnectionFW_POLL_MS;
    }
    return m_unreliable_connection.wait_for_recv_available(milliseconds);
}
bool ReliableStreamConnectionFW::iterate_retransmits(){
    WallClock now = pabb_current_time();
    WallClock next_retransmit = m_last_retransmit + pabb_milliseconds(PABB2_ReliableConnectionFW_POLL_MS);
    if (pabb_time_wrapsafe_cmplt(now, next_retransmit)){
        return false;
    }

    m_last_retransmit = now;
    return m_reliable_sender.iterate_retransmits();
}
bool ReliableStreamConnectionFW::run_events(){
    const PacketHeader* packet = m_parser.pull_bytes(m_unreliable_connection);
    if (packet == nullptr){
        return iterate_retransmits();
    }

    //  Check the packet status.
    switch (packet->magic_number){
    case PABB2_PacketParser_RESULT_VALID:
        break;
    case PABB2_PacketParser_RESULT_INVALID:
//        printf("PABB2_PacketParser_RESULT_INVALID\n");
        m_reliable_sender.send_oob_packet_empty(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_INVALID_LENGTH
        );
        return true;
    case PABB2_PacketParser_RESULT_CHECKSUM_FAIL:
//        printf("PABB2_PacketParser_RESULT_CHECKSUM_FAIL\n");
        m_reliable_sender.send_oob_packet_empty(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL
        );
//        cout << "CRC error:";
//        PacketHeader_print(packet, true);
        return true;
    default:
//        printf("Internal Error: Unrecognized packet state.\n");
        return true;
    }

//    printf("Device Received: %d\n", packet->opcode);

    //  Now handle the different opcodes.
    uint8_t opcode = packet->opcode & PABB2_CONNECTION_OPCODE_MASK;
    switch (opcode){
    case PABB2_CONNECTION_OPCODE_ASK_RESET:
        m_reliable_sender.send_oob_packet_empty(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_RESET
        );
        m_reliable_sender.reset();
        m_parser.reset();
        m_stream_coalescer.reset();
        m_stream_coalescer.push_packet(0);
        m_reset_flag = true;
        m_stream_ready = false;
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_VERSION:
        m_stream_coalescer.push_packet(packet->seqnum);
        m_reliable_sender.send_oob_packet_u32(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_VERSION,
            PABB2_CONNECTION_PROTOCOL_VERSION
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE:
        m_stream_coalescer.push_packet(packet->seqnum);
        m_reliable_sender.send_oob_packet_u16(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE,
            PABB2_MAX_INCOMING_PACKET_SIZE
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS:
        m_stream_coalescer.push_packet(packet->seqnum);
        m_reliable_sender.send_oob_packet_u8(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS,
            PABB2_StreamCoalescer_SLOTS
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES:
        m_stream_coalescer.push_packet(packet->seqnum);
        m_reliable_sender.send_oob_packet_u16(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES,
            PABB2_StreamCoalescer_BUFFER_SIZE
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA:
        m_stream_ready = true;
        if (!m_stream_coalescer.push_stream((const PacketHeaderData*)packet)){
            send_oob_info_str("Stream is full.");
            return true;
        }
        m_reliable_sender.send_oob_packet_u16(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_STREAM_DATA,
            m_stream_coalescer.free_bytes()
        );
        return true;
    case PABB2_CONNECTION_OPCODE_RET_STREAM_DATA:
        m_reliable_sender.remove(packet->seqnum);
        return true;
    default:
        m_reliable_sender.send_oob_packet_u8(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE,
            packet->opcode
        );
    }

    return true;
}




}
}
