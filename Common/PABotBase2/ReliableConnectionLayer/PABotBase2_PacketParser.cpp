/*  PABotBase2 Packet Parser
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_PacketParser.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include "PABotBase2_ConnectionDebug.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wtautological-constant-out-of-range-compare"
#endif

namespace PokemonAutomation{
namespace PABotBase2{


const PacketHeader* PacketParser::pull_bytes(
    UnreliableStreamConnectionPolling& connection,
    const uint32_t& session_id,
    WallDuration timeout
){
    const uint8_t MIN_PACKET_SIZE = sizeof(PacketHeader) + sizeof(uint32_t);

    while (true){
        uint8_t index = m_index;

        //  Already have the entire header.
        if (index >= MIN_PACKET_SIZE){
            break;
        }

        //  Read enough to finish the header.
        m_index += (uint8_t)connection.unreliable_recv(
            m_buffer + index,
            MIN_PACKET_SIZE - index,
            timeout
        );
        timeout = milliseconds_to_duration(0);

        //  Header is still incomplete.
        if (m_index < MIN_PACKET_SIZE){
//            cout << "Incomplete header: " << (int)m_index << endl;
            return NULL;
        }

        //  Valid magic byte.
        uint8_t* buffer = m_buffer;
        if (buffer[0] == PABB2_CONNECTION_MAGIC_NUMBER){
            break;
        }

        //  Search for the magic byte in the remaining buffer.
        uint8_t c = 0;
        while (true){
            c++;

            //  Magic byte never found.
            if (c == MIN_PACKET_SIZE){
                break;
            }

            //  Magic byte found.
            if (buffer[c] == PABB2_CONNECTION_MAGIC_NUMBER){
                //  Shift the buffer up so that the magic byte is at the start.
                memmove(m_buffer, m_buffer + c, MIN_PACKET_SIZE - c);
                break;
            }
        }

        m_index -= c;
    }

    //  At this point, we have a complete and valid header.

    PacketHeader* header = (PacketHeader*)m_buffer;

    uint8_t packet_bytes = header->packet_bytes;
    if (packet_bytes < MIN_PACKET_SIZE || packet_bytes > PABB2_MAX_INCOMING_PACKET_SIZE){
//        cout << "Invalid length: " << (int)packet_bytes << endl;

        //  Invalid length.
        header->magic_number = PABB2_PacketParser_RESULT_INVALID;
        m_index = 0;

        return header;
    }

    //  Valid length.

    //  Read enough to finish the packet.
    if (m_index < packet_bytes){
        uint8_t index = m_index;
        m_index += (uint8_t)connection.unreliable_recv(
            m_buffer + index,
            packet_bytes - index,
            timeout
        );
//        timeout = milliseconds_to_duration(0);
    }

    //  Packet is incomplete.
    if (m_index < packet_bytes){
//        printf("Incomplete Packet: %d\n", packet_bytes);
        return NULL;
    }

    //  Verify the CRC.

    uint32_t actual_crc = header->opcode == PABB2_CONNECTION_OPCODE_ASK_RESET
        ? 0xffffffff
        : session_id;
    pabb_crc32_buffer(&actual_crc, m_buffer, packet_bytes - sizeof(uint32_t));

    uint32_t expected_crc;
    memcpy(
        &expected_crc,
        m_buffer + packet_bytes - sizeof(uint32_t),
        sizeof(uint32_t)
    );

//    cout << actual_crc << " / " << expected_crc << endl;

    header->magic_number = actual_crc == expected_crc
        ? PABB2_PacketParser_RESULT_VALID
        : PABB2_PacketParser_RESULT_CHECKSUM_FAIL;

    m_index = 0;

    return header;
}

void PacketParser::push_bytes(
    PacketRunner& packet_runner,
    const uint32_t& session_id,
    const uint8_t* data, size_t bytes
){
//    cout << std::string((const char*)data, bytes) << endl;

//    cout << "pabb2_PacketParser_push_bytes(): " << bytes << endl;

//    cout << "push_bytes(): ";
//    PABotBase2::print_bytes(data, bytes, false);
//    cout << endl;

    const uint8_t MIN_PACKET_SIZE = sizeof(PacketHeader) + sizeof(uint32_t);

    uint8_t index = m_index;

    while (bytes > 0){
//        cout << "index = " << (int)index << ", bytes left = " << bytes << endl;
//        cout << "bytes left: " << bytes << endl;
        if (index == 0 && data[0] != PABB2_CONNECTION_MAGIC_NUMBER){
//            cout << "Skipping invalid start of packet." << endl;
            data++;
            bytes--;
            continue;
        }

        if (index < MIN_PACKET_SIZE){
//            cout << "Header is Incomplete: " << (int)index << endl;
            uint8_t bytes_needed_to_finish_header = MIN_PACKET_SIZE - index;
            if (bytes < bytes_needed_to_finish_header){
//                cout << "Not enough data to complete header: " << (int)index << endl;
                memcpy(m_buffer + index, data, bytes);
                index += (uint8_t)bytes;
                break;
            }
            memcpy(m_buffer + index, data, bytes_needed_to_finish_header);
            data += bytes_needed_to_finish_header;
            bytes -= bytes_needed_to_finish_header;
            index += bytes_needed_to_finish_header;
        }

        PacketHeader* header = (PacketHeader*)m_buffer;

        uint8_t packet_bytes = header->packet_bytes;
        if (packet_bytes < MIN_PACKET_SIZE || packet_bytes > PABB2_MAX_INCOMING_PACKET_SIZE){
//            cout << "Invalid length: " << (int)packet_bytes << endl;

            //  Invalid length.
            header->magic_number = PABB2_PacketParser_RESULT_INVALID;

        }else{
//            cout << "Valid length: " << (int)packet_bytes << endl;

            //  Valid length.

            uint8_t bytes_needed_to_finish_packet = packet_bytes - index;

//            cout << "bytes = " << (int)bytes << ", bytes_needed_to_finish_packet = " << (int)bytes_needed_to_finish_packet << endl;

            if (bytes < bytes_needed_to_finish_packet){
                memcpy(m_buffer + index, data, bytes);
                index += (uint8_t)bytes;
                break;
            }

            memcpy(m_buffer + index, data, bytes_needed_to_finish_packet);
            data += bytes_needed_to_finish_packet;
            bytes -= bytes_needed_to_finish_packet;

            //  Verify the CRC.

            uint32_t actual_crc = header->opcode == PABB2_CONNECTION_OPCODE_ASK_RESET
                ? 0xffffffff
                : session_id;
            pabb_crc32_buffer(&actual_crc, m_buffer, packet_bytes - sizeof(uint32_t));

            uint32_t expected_crc;
            memcpy(&expected_crc, m_buffer + packet_bytes - sizeof(uint32_t), sizeof(uint32_t));

//            cout << actual_crc << " / " << expected_crc << endl;

            header->magic_number = actual_crc == expected_crc
                ? PABB2_PacketParser_RESULT_VALID
                : PABB2_PacketParser_RESULT_CHECKSUM_FAIL;
        }

        packet_runner.on_packet(header);
        index = 0;
    }

    m_index = index;
}


















}
}
