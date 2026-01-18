/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PABotBase2_MessageDumper.h"

namespace PokemonAutomation{


std::string tostr(const pabb2_PacketHeader* header){
    std::string str;
    switch (header->opcode){
    case PABB2_CONNECTION_OPCODE_INVALID:
        str += "PABB2_CONNECTION_OPCODE_INVALID: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_RESET:
        str += "PABB2_CONNECTION_OPCODE_RESET: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;

    case PABB2_CONNECTION_OPCODE_QUERY_VERSION:
        str += "PABB2_CONNECTION_OPCODE_QUERY_VERSION: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_QUERY_PACKET_SIZE:
        str += "PABB2_CONNECTION_OPCODE_QUERY_PACKET_SIZE: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_QUERY_BUFFER_SLOTS:
        str += "PABB2_CONNECTION_OPCODE_QUERY_BUFFER_SLOTS: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_QUERY_BUFFER_BYTES:
        str += "PABB2_CONNECTION_OPCODE_QUERY_BUFFER_BYTES: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;

    case PABB2_CONNECTION_OPCODE_STREAM_DATA:
        str += "PABB2_CONNECTION_OPCODE_STREAM_DATA: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const pabb2_PacketHeaderData*)header)->stream_offset);
        str += ", bytes = " + std::to_string(header->packet_bytes - sizeof(pabb2_PacketHeaderData) - sizeof(uint32_t));
        return str;
    case PABB2_CONNECTION_OPCODE_STREAM_REQUEST:
        str += "PABB2_CONNECTION_OPCODE_STREAM_REQUEST: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const pabb2_PacketHeaderData*)header)->stream_offset);
        return str;

    case PABB2_CONNECTION_OPCODE_ACK:
        str += "PABB2_CONNECTION_OPCODE_ACK: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_ACK_u8:
        str += "PABB2_CONNECTION_OPCODE_ACK_u8: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const pabb2_PacketHeader_Ack_u8*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_ACK_u16:
        str += "PABB2_CONNECTION_OPCODE_ACK_u16: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const pabb2_PacketHeader_Ack_u16*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_ACK_u32:
        str += "PABB2_CONNECTION_OPCODE_ACK_u32: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const pabb2_PacketHeader_Ack_u32*)header)->data);
        return str;

    case PABB2_CONNECTION_OPCODE_INVALID_LENGTH:
        str += "PABB2_CONNECTION_OPCODE_INVALID_LENGTH: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL:
        str += "PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_INVALID_OPCODE:
        str += "PABB2_CONNECTION_OPCODE_INVALID_OPCODE: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE:
        str += "PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    }
    return str;
}


}
