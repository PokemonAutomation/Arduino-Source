/*  PABotBase2 Packet Parser
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_PacketParser_H
#define PokemonAutomation_PABotBase2_PacketParser_H

#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_Connection.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

#ifdef __cplusplus
extern "C" {
#endif



#define PABB2_MAX_INCOMING_PACKET_SIZE      20



typedef void (*pabb2_fp_PacketRunner)(void* context, const pabb2_PacketHeader* data);


#define PABB2_PacketParser_RESULT_VALID             0
#define PABB2_PacketParser_RESULT_INVALID           1
#define PABB2_PacketParser_RESULT_CHECKSUM_FAIL     2


typedef struct{
    uint8_t index;

    pabb2_fp_PacketRunner executor;
    void* executor_context;

    uint8_t buffer[PABB2_MAX_INCOMING_PACKET_SIZE];
} pabb2_PacketParser;


inline void pabb2_PacketParser_init(
    pabb2_PacketParser* self,
    pabb2_fp_PacketRunner executor,
    void* executor_context
){
    self->index = 0;
    self->executor = executor;
    self->executor_context = executor_context;
}

inline void pabb2_PacketParser_push_bytes(
    pabb2_PacketParser* self,
    const uint8_t* data, size_t bytes
){
//    cout << "pabb2_PacketParser_push_bytes(): " << bytes << endl;

    const uint8_t MIN_PACKET_SIZE = sizeof(pabb2_PacketHeader) + sizeof(uint32_t);

    uint8_t index = self->index;

    while (bytes > 0){
//        cout << "bytes left: " << bytes << endl;
        if (index == 0 && data[0] != PABB2_CONNECTION_PACKET_MAGIC_NUMBER){
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
                memcpy(self->buffer + index, data, bytes);
                break;
            }
            memcpy(self->buffer + index, data, bytes_needed_to_finish_header);
            data += bytes_needed_to_finish_header;
            bytes -= bytes_needed_to_finish_header;
            index += bytes_needed_to_finish_header;
        }

        pabb2_PacketHeader* header = (pabb2_PacketHeader*)self->buffer;

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
                memcpy(self->buffer + index, data, bytes);
                index += (uint8_t)bytes;
                break;
            }

            memcpy(self->buffer + index, data, bytes_needed_to_finish_packet);
            data += bytes_needed_to_finish_packet;
            bytes -= bytes_needed_to_finish_packet;

            uint32_t actual_crc = 0xffffffff;
            pabb_crc32_buffer(&actual_crc, self->buffer, packet_bytes - sizeof(uint32_t));

            uint32_t expected_crc;
            memcpy(&expected_crc, self->buffer + packet_bytes - sizeof(uint32_t), sizeof(uint32_t));

//            cout << actual_crc << " / " << expected_crc << endl;

            header->magic_number = actual_crc == expected_crc
                ? PABB2_PacketParser_RESULT_VALID
                : PABB2_PacketParser_RESULT_CHECKSUM_FAIL;
        }

        self->executor(self->executor_context, header);
        index = 0;
    }

    self->index = index;
}




#ifdef __cplusplus
}
#endif
#endif
