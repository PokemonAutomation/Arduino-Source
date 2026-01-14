/*  Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32/pabb_CRC32.h"
#include "ReliableStreamConnection.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



ReliableStreamConnection::ReliableStreamConnection(
    Logger& logger,
    StreamConnection& unreliable_connection,
    WallDuration retransmit_timeout
)
    : m_logger(logger)
    , m_unreliable_connection(unreliable_connection)
    , m_retransmit_timeout(retransmit_timeout)
{
    pabb2_PacketSender_init(&m_reliable_sender, this, &ReliableStreamConnection::send_raw);
    pabb2_PacketParser_init(&m_parser);
    pabb2_StreamCoalescer_init(&m_stream_coalescer);

    m_retransmit_thread = Thread([this]{ retransmit_thread(); });

    m_unreliable_connection.add_listener(*this);
}

ReliableStreamConnection::~ReliableStreamConnection(){
    m_unreliable_connection.remove_listener(*this);
    stop();
}

void ReliableStreamConnection::stop(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_stopping){
            return;
        }
        m_stopping = true;
    }
    m_cv.notify_all();
    m_retransmit_thread.join();
}



//
//  Send Path
//

size_t ReliableStreamConnection::send(const void* data, size_t bytes){
    std::lock_guard<std::mutex> lg(m_lock);
    return pabb2_PacketSender_send_stream(&m_reliable_sender, data, bytes);
}

bool ReliableStreamConnection::send_request(uint8_t opcode){
    std::lock_guard<std::mutex> lg(m_lock);
    return pabb2_PacketSender_send_packet(&m_reliable_sender, opcode, 0, nullptr);
}


void ReliableStreamConnection::send_ack(uint8_t seqnum){
    struct{
        pabb2_PacketHeader header;
        uint8_t crc[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_PACKET_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = PABB2_CONNECTION_PACKET_OPCODE_ACK;
    pabb_crc32_write_to_message(&packet, sizeof(packet));

    std::lock_guard<std::mutex> lg(m_lock);
    m_unreliable_connection.send(&packet, sizeof(packet));
}

void ReliableStreamConnection::retransmit_thread(){
    WallClock next_retransmit = current_time() + m_retransmit_timeout;
    while (true){
        std::unique_lock<std::mutex> lg(m_lock);

        m_cv.wait_until(lg, next_retransmit);

        if (m_stopping){
            break;
        }
        if (pabb2_PacketSender_size(&m_reliable_sender) == 0){
            continue;
        }
        if (current_time() < next_retransmit){
            continue;
        }

        cout << "running retransmits" << endl;

        if (!pabb2_PacketSender_iterate_retransmits(&m_reliable_sender)){
            cout << "nothing to do" << endl;
            next_retransmit = current_time() + m_retransmit_timeout;
        }else{
            cout << "did something" << endl;
        }
    }
}



//
//  Receive Path
//

void ReliableStreamConnection::on_packet(const pabb2_PacketHeader* packet){
    uint8_t status = packet->magic_number;

//    cout << "on_packet(): seqnum = " << (int)packet->seqnum << ", opcode = " << (int)packet->opcode << endl;

    switch (status){
    case PABB2_PacketParser_RESULT_VALID:
        break;
    case PABB2_PacketParser_RESULT_INVALID:
        m_logger.log(
            "[ReliableStreamConnection]: INVALID MESSAGE: Received invalid message from device.",
            COLOR_RED
        );
        return;
    case PABB2_PacketParser_RESULT_CHECKSUM_FAIL:
        m_logger.log(
            "[ReliableStreamConnection]: CHECKSUM MISMATCH: Received bad data from device.",
            COLOR_RED
        );
        return;
    default:
        m_logger.log(
            "[ReliableStreamConnection]: Internal Error: Packet parser returned invalid code: " + std::to_string(status),
            COLOR_RED
        );
        return;
    }


    if (packet->opcode == PABB2_CONNECTION_PACKET_OPCODE_STREAM_DATA){
        if (packet->packet_bytes < sizeof(pabb2_PacketHeaderData) + sizeof(uint32_t)){
            m_logger.log(
                "[ReliableStreamConnection]: Received stream packet that is too small: " + std::to_string(packet->packet_bytes),
                COLOR_RED
            );
            return;
        }
        std::lock_guard<std::mutex> lg(m_lock);
        if (pabb2_StreamCoalescer_push_stream(&m_stream_coalescer, (const pabb2_PacketHeaderData*)packet)){
            send_ack(packet->seqnum);
        }
        return;
    }


    switch (packet->opcode){
    case PABB2_CONNECTION_PACKET_OPCODE_INVALID_LENGTH:{
        m_logger.log(
            "[ReliableStreamConnection]: PABB2_CONNECTION_PACKET_OPCODE_INVALID_LENGTH: Device reported an invalid message length.",
            COLOR_RED
        );
        return;
    }
    case PABB2_CONNECTION_PACKET_OPCODE_INVALID_CHECKSUM_FAIL:{
        m_logger.log(
            "[ReliableStreamConnection]: PABB2_CONNECTION_PACKET_OPCODE_INVALID_CHECKSUM_FAIL: Device reported a checksum mismatch.",
            COLOR_RED
        );
        return;
    }
    case PABB2_CONNECTION_PACKET_OPCODE_INVALID_OPCODE:{
        m_logger.log(
            "[ReliableStreamConnection]: PABB2_CONNECTION_PACKET_OPCODE_INVALID_OPCODE: Device reported an invalid opcode.",
            COLOR_RED
        );
        return;
    }
    case PABB2_CONNECTION_PACKET_OPCODE_ACK:
//        cout << "Received ack" << endl;
        pabb2_PacketSender_remove(&m_reliable_sender, packet->seqnum);
        return;
    default:
        m_logger.log(
            "[ReliableStreamConnection]: UNKNOWN OPCODE: Device send an unknown opcode: " + std::to_string(packet->opcode),
            COLOR_RED
        );
        return;
    }
}






}
