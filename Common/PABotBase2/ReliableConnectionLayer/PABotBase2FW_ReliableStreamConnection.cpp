/*  PABotBase2 Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include <algorithm>
#include "PABotBase2_ConnectionDebug.h"
#include "PABotBase2FW_ReliableStreamConnection.h"

#ifdef PABB2_SUPPORTS_PRINTF_LOGGING
#include <stdio.h>
#endif

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
    , m_last_retransmit(current_time())
{}



bool ReliableStreamConnectionFW::enqueue_uncommitted_reliable_sends(const void* data, size_t bytes) noexcept{
    if (!m_stream_ready){
        m_reliable_sender.send_oob_packet_empty(0, PABB2_CONNECTION_OPCODE_INFO_STREAM_NOT_READY);
#ifdef PABB2_SUPPORTS_PRINTF_LOGGING
        printf("Stream not ready...\n");
#endif
        return false;
    }
    if (m_reliable_sender.enqueue_uncommitted_send_stream(data, bytes)){
        m_send_is_currently_full = false;
        return true;
    }
    if (!m_send_is_currently_full){
        m_send_is_currently_full = true;
        m_reliable_sender.send_oob_packet_empty(0, PABB2_CONNECTION_OPCODE_INFO_STREAM_SEND_FULL);
#ifdef PABB2_SUPPORTS_PRINTF_LOGGING
        printf("Send stream is full...\n");
#endif
    }
    return false;
}
void ReliableStreamConnectionFW::abort_uncommitted_reliable_sends() noexcept{
    m_reliable_sender.abort_uncommitted_send_stream();
}
void ReliableStreamConnectionFW::commit_uncommitted_reliable_sends() noexcept{
    m_reliable_sender.commit_uncommitted_send_stream();
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



bool ReliableStreamConnectionFW::run_send_events(const WallDuration& timeout){
    constexpr WallDuration POLL_RATE = milliseconds_to_duration(PABB2_ReliableConnectionFW_POLL_MS);

    WallClock now = current_time();
    WallClock next_retransmit = m_last_retransmit + POLL_RATE;
    if (wrapsafe_cmplt(now, next_retransmit)){
        return false;
    }

    m_last_retransmit = now;
    return m_reliable_sender.iterate_retransmits();
}
bool ReliableStreamConnectionFW::run_recv_events(const WallDuration& timeout){
    //  If we have unacked sends, we cap the wait time since those may need to
    //  be retransmitted.
    static constexpr WallDuration POLL_RATE = milliseconds_to_duration(PABB2_ReliableConnectionFW_POLL_MS);
    const WallDuration& adjusted_timeout = m_reliable_sender.slots_used() != 0 && timeout > POLL_RATE
        ? POLL_RATE
        : timeout;

    const PacketHeader* header = m_parser.pull_bytes(
        m_unreliable_connection,
        m_reliable_sender.session_id(),
        adjusted_timeout
    );
    if (header == nullptr){
        return false;
    }

    m_packets_received++;

    //  Check the packet status.
    switch (header->magic_number){
    case PABB2_PacketParser_RESULT_VALID:
        break;
    case PABB2_PacketParser_RESULT_INVALID:
//        printf("PABB2_PacketParser_RESULT_INVALID\n");
        m_reliable_sender.send_oob_packet_empty(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_INVALID_LENGTH
        );
        return true;
    case PABB2_PacketParser_RESULT_CHECKSUM_FAIL:
//        printf("PABB2_PacketParser_RESULT_CHECKSUM_FAIL\n");
        m_reliable_sender.send_oob_packet_empty(
            header->seqnum,
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
    uint8_t opcode = header->opcode & PABB2_CONNECTION_OPCODE_MASK;
    switch (opcode){
    case PABB2_CONNECTION_OPCODE_ASK_RESET:{
        if (header->packet_bytes < sizeof(PacketHeader_u32)){
            return true;
        }

        const PacketHeader_u32* packet = (const PacketHeader_u32*)header;

#ifdef PABB2_SUPPORTS_PRINTF_LOGGING
        printf("Resetting to session ID: %zx\n", (size_t)packet->data);
#endif
        m_stream_ready = false;
        m_send_is_currently_full = false;
        m_reliable_sender.reset(packet->data);
        m_parser.reset();
        m_stream_coalescer.reset();
        m_stream_coalescer.push_packet(0);
#ifdef PABB2_ENABLE
        issue_reset_to_all();
#endif
        m_reliable_sender.send_oob_packet_empty(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_RET_RESET
        );
        return true;
    }
    case PABB2_CONNECTION_OPCODE_ASK_VERSION:
        m_stream_coalescer.push_packet(header->seqnum);
        m_reliable_sender.send_oob_packet_u32(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_RET_VERSION,
            PABB2_CONNECTION_PROTOCOL_VERSION
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE:
        m_stream_coalescer.push_packet(header->seqnum);
        m_reliable_sender.send_oob_packet_u16(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE,
            PABB2_MAX_INCOMING_PACKET_SIZE
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS:
        m_stream_coalescer.push_packet(header->seqnum);
        m_reliable_sender.send_oob_packet_u8(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS,
            PABB2_StreamCoalescer_SLOTS
        );
        return true;
    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES:
        m_stream_coalescer.push_packet(header->seqnum);
        m_reliable_sender.send_oob_packet_u16(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES,
            PABB2_StreamCoalescer_BUFFER_SIZE
        );
        return true;

    case PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA:
        m_stream_ready = true;
        if (!m_stream_coalescer.push_stream((const PacketHeaderData*)header)){
            send_oob_info_label_u32("Push Stream Failed", m_stream_coalescer.free_bytes());
            return true;
        }
        m_reliable_sender.send_oob_packet_u16(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_RET_STREAM_DATA,
            m_stream_coalescer.free_bytes()
        );
        return true;
    case PABB2_CONNECTION_OPCODE_RET_STREAM_DATA:
        m_reliable_sender.remove(header->seqnum);
        return true;
    default:
        m_reliable_sender.send_oob_packet_u8(
            header->seqnum,
            PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE,
            header->opcode
        );
    }

    return true;
}




}
}
