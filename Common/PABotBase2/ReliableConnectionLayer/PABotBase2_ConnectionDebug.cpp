/*  PABotBase2 Connection Sender (Debug)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_PacketSender.h"
#include "PABotBase2_StreamCoalescer.h"
#include "PABotBase2_ConnectionDebug.h"

namespace PokemonAutomation{
namespace PABotBase2{


void print_bytes(const void* data, size_t bytes, bool ascii){
    const char* ptr = (const char*)data;

    if (ascii){
        for (size_t c = 0; c < bytes; c++){
            std::cout << ptr[c];
        }
        return;
    }

    const char HEX[] = "0123456789abcdef";
    for (size_t c = 0; c < bytes; c++){
        uint8_t byte = ptr[c];
        if (32 <= byte && byte <= 126){
            std::cout << HEX[byte >> 4] << HEX[byte & 0xf] << "(" << byte << ") ";
        }else{
            std::cout << HEX[byte >> 4] << HEX[byte & 0xf] << " ";
        }
    }
}

void PacketHeader_print(const PacketHeader* packet, bool ascii){
    uint8_t packet_bytes = packet->packet_bytes;
    std::cout << "{" << std::endl;
    std::cout << "    magic:  0x" << std::hex << (int)packet->magic_number << std::dec << std::endl;
    std::cout << "    seqnum: " << (int)packet->seqnum << std::endl;
    std::cout << "    bytes:  " << (int)packet->packet_bytes << std::endl;
    std::cout << "    opcode: 0x" << std::hex << (int)packet->opcode << std::dec << std::endl;
    if (packet_bytes < sizeof(PacketHeader) + sizeof(uint32_t)){
        std::cout << "    ** invalid length **\n}" << std::endl;
        return;
    }

    uint32_t expected_crc32;
    memcpy(
        &expected_crc32,
        (const uint8_t*)packet + packet_bytes - sizeof(uint32_t),
        sizeof(uint32_t)
    );
    uint32_t actual_crc32 = 0xffffffff;
    uint8_t magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    pabb_crc32_buffer(&actual_crc32, &magic_number, 1);
    pabb_crc32_buffer(&actual_crc32, (const uint8_t*)packet + 1, packet_bytes - sizeof(uint32_t) - 1);
    std::cout << "    CRC32:  0x" << std::hex << actual_crc32 << " / 0x" << expected_crc32 << std::dec;
    if (actual_crc32 != expected_crc32){
        std::cout << "    (** mismatch **)";
    }
    std::cout << std::endl;

    switch (packet->opcode){
    case PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA:{
        const size_t OVERHEAD = sizeof(PacketHeaderData) + sizeof(uint32_t);
        const PacketHeaderData* stream_packet = (const PacketHeaderData*)packet;
        if (packet_bytes < OVERHEAD){
            std::cout << "    ** invalid length **" << std::endl;
            break;
        }
        std::cout << "    offset: " << stream_packet->stream_offset << std::endl;
        std::cout << "    data:   ";
        print_bytes(stream_packet + 1, packet_bytes - OVERHEAD, ascii);
        std::cout << std::endl;
        break;
    }
    default:
        std::cout << "    (unknown opcode)" << std::endl;
    }
    std::cout << "}" << std::endl;
}
bool PacketHeader_check(const PacketHeader* packet, bool ascii){
    uint8_t packet_bytes = packet->packet_bytes;

    uint32_t expected_crc32;
    memcpy(
        &expected_crc32,
        (const uint8_t*)packet + packet_bytes - sizeof(uint32_t),
        sizeof(uint32_t)
    );
    uint32_t actual_crc32 = 0xffffffff;
    uint8_t magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    pabb_crc32_buffer(&actual_crc32, &magic_number, 1);
    pabb_crc32_buffer(&actual_crc32, (const uint8_t*)packet + 1, packet_bytes - sizeof(uint32_t) - 1);
    if (actual_crc32 == expected_crc32){
        return true;
    }

    if (false){
        std::cout << "PacketHeader_check(): CRC mismatch" << std::endl;
        PacketHeader_print(packet, ascii);
//        system("pause");
    }
    return false;
}


void PacketSender::print(bool ascii) const{
    std::cout << "---- ConnectionSender ---- (Start)" << std::endl;
    std::cout << "Slot Head:             " << (int)m_slot_head << std::endl;
    std::cout << "Slot Tail:             " << (int)m_slot_tail << std::endl;
    std::cout << "Slot Tail (resv):      " << (int)m_slot_tail_uncommitted << std::endl;
    std::cout << "Buffer Head:           " << m_buffer_head << std::endl;
    std::cout << "Buffer Tail:           " << m_buffer_tail << std::endl;
    std::cout << "Buffer Tail (resv):    " << m_buffer_tail_uncommitted << std::endl;
    std::cout << "Stream Offset:         " << m_stream_offset << std::endl;
    std::cout << "Stream Offset (resv):  " << m_stream_offset_uncommitted << std::endl;
    std::cout << "Retransmit Seqnum:     " << (int)m_retransmit_seqnum << std::endl;
    for (uint8_t seqnum = m_slot_head; seqnum != m_slot_tail; seqnum++){
        size_t offset = m_offsets[seqnum & SLOTS_MASK];
        std::cout << "Offset: " << offset << std::endl;
        PacketHeader_print((const PacketHeader*)(m_buffer + offset), ascii);
    }
    std::cout << "---- ConnectionSender ---- (End)" << std::endl;
}


void StreamCoalescer::print(bool ascii) const{
    std::cout << "---- StreamCoalescer ---- (Start)" << std::endl;
    std::cout << "Slot Head:         " << (int)m_slot_head << std::endl;
    std::cout << "Slot Tail:         " << (int)m_slot_tail << std::endl;
    std::cout << "Stream Free:       " << m_stream_free << std::endl;
    std::cout << "Stream Head:       " << m_stream_head << std::endl;
    std::cout << "Stream Tail:       " << m_stream_tail << std::endl;
    for (uint8_t seqnum = m_slot_head; seqnum != m_slot_tail; seqnum++){
        uint8_t index = seqnum & SLOTS_MASK;
        uint8_t size = m_lengths[index];
        std::cout << "    [" << (int)seqnum << "]: ";
        if (size == 0){
            std::cout << std::endl;
            continue;
        }
        if (size == 0xff){
            std::cout << " non-stream" << std::endl;
            continue;
        }

        uint16_t offset_e = m_end_offsets[index];
        uint16_t offset_s = offset_e - size;
        std::cout << "[" << offset_s << ":" << offset_e << "] => ";

        offset_s &= BUFFER_MASK;
        offset_e &= BUFFER_MASK;

        if (offset_s < offset_e){
            print_bytes(m_buffer + offset_s, size, ascii);
        }else{
        uint8_t block = (uint8_t)(PABB2_StreamCoalescer_BUFFER_SIZE - offset_s);
            print_bytes(m_buffer + offset_s, block, ascii);
            print_bytes(m_buffer, size - block, ascii);
        }
        std::cout << std::endl;
    }
    std::cout << "---- StreamCoalescer ---- (End)" << std::endl;
}










}
}
