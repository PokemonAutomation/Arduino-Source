/*  PABotBase2 Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include <stdio.h>
#include "PABotBase2_ConnectionDebug.h"
#include "PABotBase2_ReliableStreamConnectionFW.h"

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


ReliableStreamConnectionFW::ReliableStreamConnectionFW(StreamConnection& unreliable_connection)
    : m_reliable_sender(unreliable_connection, (uint8_t)(PABB2_MAX_INCOMING_PACKET_SIZE % 256))
    , m_parser(unreliable_connection)
{}

bool ReliableStreamConnectionFW::run_events(){
    const PacketHeader* packet = m_parser.pull_bytes();
    if (packet == NULL){
        return m_reliable_sender.iterate_retransmits();
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
    switch (packet->opcode){
    case PABB2_CONNECTION_OPCODE_ASK_RESET:
        m_reliable_sender.send_oob_packet_empty(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_RESET
        );
        m_reliable_sender.reset();
        m_parser.reset();
        m_stream_coalescer.reset();
        m_stream_coalescer.push_packet(0);
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
//        printf("Device: PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA\n");
        if (m_stream_coalescer.push_stream((const PacketHeaderData*)packet)){
//            printf("Device: Succeeded push.\n");
            m_reliable_sender.send_oob_packet_u16(
                packet->seqnum,
                PABB2_CONNECTION_OPCODE_RET_STREAM_DATA,
                m_stream_coalescer.bytes_available()
            );
        }else{
//            printf("Device: Failed to push.\n");
//            m_stream_coalescer.print(true);
        }
//        fflush(stdout);
        return true;
    default:
        m_reliable_sender.send_oob_packet_empty(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE
        );
    }

    return true;
}




}
}
