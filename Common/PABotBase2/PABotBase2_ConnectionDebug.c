/*  PABotBase2 Connection Sender (Debug)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdio.h>
#include "Common/CRC32/pabb_CRC32.h"
#include "PABotBase2_ConnectionDebug.h"


void pabb2_print_bytes(const void* data, size_t bytes, bool ascii){
    const char* ptr = (const char*)data;

    if (ascii){
        for (size_t c = 0; c < bytes; c++){
            printf("%c", ptr[c]);
        }
        return;
    }

    const char HEX[] = "0123456789abcdef";
    for (size_t c = 0; c < bytes; c++){
        uint8_t byte = ptr[c];
        if (32 <= byte && byte <= 126){
            printf("%c%c(%c) ", HEX[byte >> 4], HEX[byte & 0xf], (char)byte);
        }else{
            printf("%c%c ", HEX[byte >> 4], HEX[byte & 0xf]);
        }
    }
}

void pabb2_PacketHeader_print(const pabb2_PacketHeader* packet, bool ascii){
    uint8_t packet_bytes = packet->packet_bytes;
    printf("{\n");
    printf("    magic:  0x%x\n", packet->magic_number);
    printf("    seqnum: %u\n", packet->seqnum);
    printf("    bytes:  %u\n", packet->packet_bytes);
    printf("    opcode: 0x%u\n", packet->opcode);
    if (packet_bytes < sizeof(pabb2_PacketHeader) + sizeof(uint32_t)){
        printf("    ** invalid length **\n}\n");
        return;
    }

    uint32_t expected_crc32;
    memcpy(
        &expected_crc32,
        (const uint8_t*)packet + packet_bytes - sizeof(uint32_t),
        sizeof(uint32_t)
    );
    uint32_t actual_crc32 = 0xffffffff;
    uint8_t magic_number = PABB2_CONNECTION_PACKET_MAGIC_NUMBER;
    pabb_crc32_buffer(&actual_crc32, &magic_number, 1);
    pabb_crc32_buffer(&actual_crc32, (const uint8_t*)packet + 1, packet_bytes - sizeof(uint32_t) - 1);
    printf("    CRC32:  0x%u / 0x%u", actual_crc32, expected_crc32);
    if (actual_crc32 != expected_crc32){
        printf("    (** mismatch **)");
    }
    printf("\n");

    switch (packet->opcode){
    case PABB2_CONNECTION_PACKET_OPCODE_STREAM_DATA:{
        const size_t OVERHEAD = sizeof(pabb2_PacketHeaderData) + sizeof(uint32_t);
        const pabb2_PacketHeaderData* stream_packet = (const pabb2_PacketHeaderData*)packet;
        if (packet_bytes < OVERHEAD){
            printf("    ** invalid length **");
            break;
        }
        printf("    offset: %u\n", stream_packet->stream_offset);
        printf("    data:   ");
        pabb2_print_bytes(stream_packet + 1, packet_bytes - OVERHEAD, ascii);
        printf("\n");
        break;
    }
    default:
        printf("    (unknown opcode)\n");
    }
    printf("}\n");
}

void pabb2_PacketSender_print(const pabb2_PacketSender* self, bool ascii){
    printf("---- ConnectionSender ---- (Start)\n");
    printf("Slot Head:         %u\n", self->slot_head);
    printf("Slot Tail:         %u\n", self->slot_tail);
    printf("Buffer Head:       %zu\n", self->buffer_head);
    printf("Buffer Tail:       %zu\n", self->buffer_tail);
    printf("Stream Offset:     %u\n", self->stream_offset);
    printf("Retransmit Seqnum: %u\n", self->retransmit_seqnum);
    for (uint8_t seqnum = self->slot_head; seqnum != self->slot_tail; seqnum++){
        size_t offset = ~self->offsets[seqnum & PABB2_ConnectionSender_SLOTS_MASK];
        printf("Offset: %zu\n", offset);
        pabb2_PacketHeader_print((const pabb2_PacketHeader*)(self->buffer + offset), ascii);
    }
    printf("---- ConnectionSender ---- (End)\n");
    fflush(stdout);
}


void pabb2_StreamCoalescer_print(const pabb2_StreamCoalescer* self, bool ascii){
    printf("---- StreamCoalescer ---- (Start)\n");
    printf("Slot Head:         %u\n", self->slot_head);
    printf("Slot Tail:         %u\n", self->slot_tail);
    printf("Stream Head:       %u\n", self->stream_head);
    printf("Stream Tail:       %u\n", self->stream_tail);
    for (uint8_t seqnum = self->slot_head; seqnum != self->slot_tail; seqnum++){
        uint8_t index = seqnum & PABB2_StreamCoalescer_SLOTS_MASK;
        uint8_t size = self->lengths[index];
        printf("    [%u]: ", seqnum);
        if (size == 0){
            printf(" \n");
            continue;
        }
        if (size == 255){
            printf(" non-stream\n");
            continue;
        }

        uint16_t offset_s = self->offsets[index];
        uint16_t offset_e = offset_s + size;
        printf("[%u:%u] => ", offset_s, offset_e);

        offset_s &= PABB2_StreamCoalescer_BUFFER_MASK;
        offset_e &= PABB2_StreamCoalescer_BUFFER_MASK;

        if (offset_s < offset_e){
            pabb2_print_bytes(self->buffer + offset_s, size, ascii);
        }else{
        uint8_t block = (uint8_t)(PABB2_StreamCoalescer_BUFFER_SIZE - offset_s);
            pabb2_print_bytes(self->buffer + offset_s, block, ascii);
            pabb2_print_bytes(self->buffer, size - block, ascii);
        }
        printf("\n");
    }
    printf("---- StreamCoalescer ---- (End)\n");
    fflush(stdout);
}











