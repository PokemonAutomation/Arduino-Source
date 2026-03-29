/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "PABotBase2CC_MessageDumper.h"

namespace PokemonAutomation{
namespace PABotBase2{


std::string tostr(const PacketHeader* header){
    //  TODO: Make sure packets are large enough before reading them.

    std::string str;
    if (header->opcode & PABB2_CONNECTION_RETRANSMIT_FLAG){
        str += "(Retransmit) ";
    }
    switch (header->opcode & PABB2_CONNECTION_OPCODE_MASK){
    case PABB2_CONNECTION_OPCODE_INVALID:
        str += "PABB2_CONNECTION_OPCODE_INVALID: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;

    case PABB2_CONNECTION_OPCODE_ASK_RESET:
        str += "PABB2_CONNECTION_OPCODE_ASK_RESET: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_RET_RESET:
        str += "PABB2_CONNECTION_OPCODE_RET_RESET: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;

    case PABB2_CONNECTION_OPCODE_ASK_VERSION:
        str += "PABB2_CONNECTION_OPCODE_ASK_VERSION: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_RET_VERSION:
        str += "PABB2_CONNECTION_OPCODE_RET_VERSION: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", version = " + std::to_string(((const PacketHeader_u32*)header)->data);
        return str;

    case PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE:
        str += "PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE:
        str += "PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", bytes = " + std::to_string(((const PacketHeader_u16*)header)->data);
        return str;

    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS:
        str += "PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS:
        str += "PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", slots = " + std::to_string(((const PacketHeader_u8*)header)->data);
        return str;

    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES:
        str += "PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES:
        str += "PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", bytes = " + std::to_string(((const PacketHeader_u16*)header)->data);
        return str;

    case PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA:
        str += "PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const PacketHeaderData*)header)->stream_offset);
        str += ", bytes = " + std::to_string(header->packet_bytes - sizeof(PacketHeaderData) - sizeof(uint32_t));
        return str;
    case PABB2_CONNECTION_OPCODE_RET_STREAM_DATA:
        str += "PABB2_CONNECTION_OPCODE_RET_STREAM_REQUEST: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_ASK_STREAM_REQUEST:
        str += "PABB2_CONNECTION_OPCODE_RET_STREAM: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", offset = " + std::to_string(((const PacketHeaderData*)header)->stream_offset);
        return str;

    case PABB2_CONNECTION_OPCODE_INFO:
        str += "PABB2_CONNECTION_OPCODE_INFO: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_U8:
        str += "PABB2_CONNECTION_OPCODE_INFO_U8: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", data = " + std::to_string(((const PacketHeader_u8*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_U16:
        str += "PABB2_CONNECTION_OPCODE_INFO_U16: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", data = " + std::to_string(((const PacketHeader_u16*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_H32:
        str += "PABB2_CONNECTION_OPCODE_INFO_H32: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", data = " + tostr_hex(((const PacketHeader_u32*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_U32:
        str += "PABB2_CONNECTION_OPCODE_INFO_U32: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", data = " + std::to_string(((const PacketHeader_u32*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_I32:
        str += "PABB2_CONNECTION_OPCODE_INFO_I32: seqnum = ";
        str += std::to_string(header->seqnum);
        str += ", data = " + std::to_string((int32_t)((const PacketHeader_u32*)header)->data);
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_STR:
        str += "PABB2_CONNECTION_OPCODE_INFO_STR: ";
        str += std::string(
            (const char*)(header + 1),
            header->packet_bytes - sizeof(PacketHeader) - sizeof(uint32_t)
        );
        return str;
    case PABB2_CONNECTION_OPCODE_INFO_LABEL_H32:{
        const PacketHeader_u32* packet = (const PacketHeader_u32*)header;
        str += "PABB2_CONNECTION_OPCODE_INFO_LABEL_H32: ";
        str += std::string(
            (const char*)(packet + 1),
            header->packet_bytes - sizeof(PacketHeader_u32) - sizeof(uint32_t)
        );
        str += ": " + tostr_hex(packet->data);
        return str;
    }
    case PABB2_CONNECTION_OPCODE_INFO_LABEL_U32:{
        const PacketHeader_u32* packet = (const PacketHeader_u32*)header;
        str += "PABB2_CONNECTION_OPCODE_INFO_LABEL_U32: ";
        str += std::string(
            (const char*)(packet + 1),
            header->packet_bytes - sizeof(PacketHeader_u32) - sizeof(uint32_t)
        );
        str += ": " + std::to_string(packet->data);
        return str;
    }
    case PABB2_CONNECTION_OPCODE_INFO_LABEL_I32:{
        const PacketHeader_u32* packet = (const PacketHeader_u32*)header;
        str += "PABB2_CONNECTION_OPCODE_INFO_LABEL_I32: ";
        str += std::string(
            (const char*)(packet + 1),
            header->packet_bytes - sizeof(PacketHeader_u32) - sizeof(uint32_t)
        );
        str += ": " + std::to_string((int32_t)packet->data);
        return str;
    }

    case PABB2_CONNECTION_OPCODE_INVALID_LENGTH:
        str += "PABB2_CONNECTION_OPCODE_INVALID_LENGTH: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL:
        str += "PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    case PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE:
        str += "PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE: seqnum = ";
        str += std::to_string(header->seqnum);
        return str;
    }
    return "Unknown Packet Opcode: 0x" + tostr_hex(header->opcode);
}

std::string tostr(const MessageHeader* header){
    //  TODO: Make sure packets are large enough before reading them.

    std::string str;
    switch (header->opcode){
    case PABB2_MESSAGE_OPCODE_INVALID:
        str += "PABB2_MESSAGE_OPCODE_INVALID: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_LOG_STRING:
        str += "PABB2_MESSAGE_OPCODE_LOG_STRING: id = ";
        str += std::to_string(header->id);
        str += ": " + std::string(
            (const char*)(header + 1),
            header->message_bytes - sizeof(MessageHeader)
        );
        return str;
    case PABB2_MESSAGE_OPCODE_LOG_LABEL_H32:{
        const Message_u32* message = (const Message_u32*)header;
        str += "PABB2_MESSAGE_OPCODE_LOG_LABEL_U32: id = ";
        str += std::to_string(message->id);
        str += ": " + std::string(
            (const char*)(message + 1),
            message->message_bytes - sizeof(Message_u32)
        );
        str += ": ";
        str += tostr_hex(message->data);
        return str;
    }
    case PABB2_MESSAGE_OPCODE_LOG_LABEL_U32:{
        const Message_u32* message = (const Message_u32*)header;
        str += "PABB2_MESSAGE_OPCODE_LOG_LABEL_U32: id = ";
        str += std::to_string(message->id);
        str += ": " + std::string(
            (const char*)(message + 1),
            message->message_bytes - sizeof(Message_u32)
        );
        str += ": ";
        str += std::to_string((uint32_t)message->data);
        return str;
    }
    case PABB2_MESSAGE_OPCODE_LOG_LABEL_I32:{
        const Message_u32* message = (const Message_u32*)header;
        str += "PABB2_MESSAGE_OPCODE_LOG_LABEL_U32: id = ";
        str += std::to_string(message->id);
        str += ": " + std::string(
            (const char*)(message + 1),
            message->message_bytes - sizeof(Message_u32)
        );
        str += ": ";
        str += std::to_string((int32_t)message->data);
        return str;
    }

    case PABB2_MESSAGE_OPCODE_RET:
        str += "PABB2_MESSAGE_OPCODE_RET: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_REQUEST_DROPPED:
        str += "PABB2_MESSAGE_OPCODE_REQUEST_DROPPED: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_RET_U32:
        str += "PABB2_MESSAGE_OPCODE_RET_U32: id = ";
        str += std::to_string(header->id);
        str += ", data = " + std::to_string(((const Message_u32*)header)->data);
        return str;
    case PABB2_MESSAGE_OPCODE_RET_DATA:
        str += "PABB2_MESSAGE_OPCODE_RET_DATA: id = ";
        str += std::to_string(header->id);
        str += ", bytes = " + std::to_string(header->message_bytes - sizeof(MessageHeader));
        return str;

    case PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION:
        str += "PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION:
        str += "PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER:
        str += "PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_DEVICE_NAME:
        str += "PABB2_MESSAGE_OPCODE_DEVICE_NAME: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_CONTROLLER_LIST:
        str += "PABB2_MESSAGE_OPCODE_CONTROLLER_LIST: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_CQ_CAPACITY:
        str += "PABB2_MESSAGE_OPCODE_CQ_CAPACITY: id = ";
        str += std::to_string(header->id);
        return str;

    case PABB2_MESSAGE_OPCODE_READ_CONTROLLER_MODE:
        str += "PABB2_MESSAGE_OPCODE_READ_CONTROLLER_MODE: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE:{
        const Message_u32* message = (const Message_u32*)header;
        str += "PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE: id = ";
        str += std::to_string(message->id);
        str += ", controller = " + std::to_string(message->data);
        return str;
    }
    case PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER:{
        const Message_u32* message = (const Message_u32*)header;
        str += "PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER: id = ";
        str += std::to_string(message->id);
        str += ", controller = " + std::to_string(message->data);
        return str;
    }
    case PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS:
        str += "PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS:
        str += "PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS: id = ";
        str += std::to_string(header->id);
        return str;
    case PABB2_MESSAGE_OPCODE_REQUEST_STATUS:
        str += "PABB2_MESSAGE_OPCODE_REQUEST_STATUS: id = ";
        str += std::to_string(header->id);
        return str;

    case PABB2_MESSAGE_OPCODE_CQ_CANCEL:
        str += "PABB2_MESSAGE_OPCODE_CQ_CANCEL";
        return str;
    case PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT:
        str += "PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT:";
        return str;
    case PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED:{
        const Message_u32* message = (const Message_u32*)header;
        str += "PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED: id = ";
        str += std::to_string(message->id);
        str += ", timestamp = ";
        str += std::to_string(message->data);
        return str;
    }
    }


    return "Unknown Message Opcode: 0x" + tostr_hex(header->opcode);
}





}
}
