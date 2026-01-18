/*  PABotBase2 Packet Parser
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_PacketParser.h"


const pabb2_PacketHeader* pabb2_PacketParser_pull_bytes(
    pabb2_PacketParser* self,
    void* context, pabb2_fp_StreamRecv recv
){
    const uint8_t MIN_PACKET_SIZE = sizeof(pabb2_PacketHeader) + sizeof(uint32_t);

    while (true){
        uint8_t index = self->index;

        //  Already have the entire header.
        if (index >= MIN_PACKET_SIZE){
            break;
        }

        //  Read enough to finish the header.
        self->index += (uint8_t)recv(context, self->buffer + index, MIN_PACKET_SIZE - index);

        //  Header is still incomplete.
        if (self->index < MIN_PACKET_SIZE){
            return NULL;
        }

        //  Valid magic byte.
        uint8_t* buffer = self->buffer;
        if (buffer[0] == PABB2_CONNECTION_MAGIC_NUMBER){
            break;
        }

        //  Search for the magic byte in the remaining buffer.
        uint8_t c = 0;
        while (true){
            c++;

            //  Magic byte never found.
            if (c == MIN_PACKET_SIZE){
                self->index = 0;
                continue;
            }

            //  Magic byte found.
            if (buffer[c] == PABB2_CONNECTION_MAGIC_NUMBER){
                break;
            }
        }

        //  Shift the buffer up so that the magic byte is at the start.
        memmove(self->buffer, self->buffer + c, MIN_PACKET_SIZE - c);

        break;
    }

    //  At this point, we have a complete and valid header.

    pabb2_PacketHeader* header = (pabb2_PacketHeader*)self->buffer;

    uint8_t packet_bytes = header->packet_bytes;
    if (packet_bytes < MIN_PACKET_SIZE || packet_bytes > PABB2_MAX_INCOMING_PACKET_SIZE){
//        cout << "Invalid length: " << (int)packet_bytes << endl;

        //  Invalid length.
        header->magic_number = PABB2_PacketParser_RESULT_INVALID;
        self->index = 0;

        return header;
    }

    //  Valid length.

    //  Read enough to finish the packet.
    if (self->index < packet_bytes){
        uint8_t index = self->index;
        self->index += (uint8_t)recv(
            context,
            self->buffer + index,
            packet_bytes - index
        );
    }

    //  Packet is incomplete.
    if (self->index < packet_bytes){
        return NULL;
    }

    //  Verify the CRC.

    uint32_t actual_crc = 0xffffffff;
    pabb_crc32_buffer(&actual_crc, self->buffer, packet_bytes - sizeof(uint32_t));

    uint32_t expected_crc;
    memcpy(
        &expected_crc,
        self->buffer + packet_bytes - sizeof(uint32_t),
        sizeof(uint32_t)
    );

//    cout << actual_crc << " / " << expected_crc << endl;

    header->magic_number = actual_crc == expected_crc
        ? PABB2_PacketParser_RESULT_VALID
        : PABB2_PacketParser_RESULT_CHECKSUM_FAIL;

    self->index = 0;

    return header;
}

void pabb2_PacketParser_push_bytes(
    pabb2_PacketParser* self,
    void* context, pabb2_fp_PacketRunner packet_runner,
    const uint8_t* data, size_t bytes
){
//    cout << "pabb2_PacketParser_push_bytes(): " << bytes << endl;

    const uint8_t MIN_PACKET_SIZE = sizeof(pabb2_PacketHeader) + sizeof(uint32_t);

    uint8_t index = self->index;

    while (bytes > 0){
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

            //  Verify the CRC.

            uint32_t actual_crc = 0xffffffff;
            pabb_crc32_buffer(&actual_crc, self->buffer, packet_bytes - sizeof(uint32_t));

            uint32_t expected_crc;
            memcpy(&expected_crc, self->buffer + packet_bytes - sizeof(uint32_t), sizeof(uint32_t));

//            cout << actual_crc << " / " << expected_crc << endl;

            header->magic_number = actual_crc == expected_crc
                ? PABB2_PacketParser_RESULT_VALID
                : PABB2_PacketParser_RESULT_CHECKSUM_FAIL;
        }

        packet_runner(context, header);
        index = 0;
    }

    self->index = index;
}



















