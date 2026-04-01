/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
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



void MessageLogger::add_message(
    uint8_t opcode,
    bool always_print,
    std::string(*tostr)(const MessageHeader*)
){
    auto ret = m_converters.emplace(opcode, Converter{always_print, tostr});
    if (ret.second){
        return;
    }
    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Duplicate opcode: " + std::to_string(opcode)
    );
}

MessageLogger::MessageLogger(){
    add_message(
        PABB2_MESSAGE_OPCODE_INVALID,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_INVALID";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_LOG_STRING,
        true,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_LOG_STRING";
            str += ": " + std::string(
                (const char*)(message + 1),
                message->message_bytes - sizeof(Message_u32)
            );
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_LOG_LABEL_H32,
        true,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_LOG_LABEL_H32";
            str += ": " + std::string(
                (const char*)(message + 1),
                message->message_bytes - sizeof(Message_u32)
            );
            str += ": " + tostr_hex(message->data);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_LOG_LABEL_U32,
        true,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_LOG_LABEL_U32";
            str += ": " + std::string(
                (const char*)(message + 1),
                message->message_bytes - sizeof(Message_u32)
            );
            str += ": " + std::to_string(message->data);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_LOG_LABEL_I32,
        true,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_LOG_LABEL_I32";
            str += ": " + std::string(
                (const char*)(message + 1),
                message->message_bytes - sizeof(Message_u32)
            );
            str += ": " + std::to_string((int32_t)message->data);
            return str;
        }
    );


    add_message(
        PABB2_MESSAGE_OPCODE_RET,
        false,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_RET";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_REQUEST_DROPPED,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_REQUEST_DROPPED";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_RET_U32,
        false,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_RET_U32";
            str += ": id = " + std::to_string(header->id);
            str += ", data = " + std::to_string(message->data);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_RET_DATA,
        false,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_RET_DATA";
            str += ": id = " + std::to_string(header->id);
            str += ", bytes = " + std::to_string(header->message_bytes - sizeof(MessageHeader));
            return str;
        }
    );


    add_message(
        PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_DEVICE_NAME,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_DEVICE_NAME";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_CONTROLLER_LIST,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_CONTROLLER_LIST";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_CQ_CAPACITY,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_CQ_CAPACITY";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );


    add_message(
        PABB2_MESSAGE_OPCODE_READ_CONTROLLER_MODE,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_READ_CONTROLLER_MODE";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE,
        true,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE";
            str += ": id = " + std::to_string(header->id);
            str += ", controller = " + std::to_string(message->data);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER,
        true,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER";
            str += ": id = " + std::to_string(header->id);
            str += ", controller = " + std::to_string(message->data);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS,
        true,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_CONTROLLER_MAC_ADDRESS";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS,
        false,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_PAIRED_MAC_ADDRESS";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_REQUEST_STATUS,
        false,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_REQUEST_STATUS";
            str += ": id = " + std::to_string(header->id);
            return str;
        }
    );


    add_message(
        PABB2_MESSAGE_OPCODE_CQ_CANCEL,
        false,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_CQ_CANCEL";
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT,
        false,
        [](const MessageHeader* header){
            std::string str = "PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT";
            return str;
        }
    );
    add_message(
        PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED,
        false,
        [](const MessageHeader* header){
            const Message_u32* message = (const Message_u32*)header;
            std::string str = "PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED";
            str += ": id = " + std::to_string(header->id);
            str += ", timestamp = " + std::to_string(message->data);
            return str;
        }
    );
}

std::string MessageLogger::to_str(const MessageHeader* message){
    uint8_t opcode = message->opcode;
    auto iter = m_converters.find(opcode);
    if (iter == m_converters.end()){
        return "(0x" + tostr_hex(opcode) + "): Unknown Opcode";
    }
    return "(0x" + tostr_hex(opcode) + ") " + iter->second.converter(message);
}
void MessageLogger::log_send(
    Logger& logger,
    bool always_log,
    const MessageHeader* message,
    Color color
){
    auto iter = m_converters.find(message->opcode);
    if (iter == m_converters.end()){
        logger.log(
            "[MLC]: Sending: (0x" + tostr_hex(message->opcode) + ") Unknown Opcode",
            COLOR_RED
        );
        return;
    }
    if (always_log || iter->second.always_log){
        logger.log(
            "[MLC]: Sending: (0x" + tostr_hex(message->opcode) + ") " + iter->second.converter(message),
            color
        );
    }
}
void MessageLogger::log_recv(
    Logger& logger,
    bool always_log,
    const MessageHeader* message,
    Color color
){
    auto iter = m_converters.find(message->opcode);
    if (iter == m_converters.end()){
        logger.log(
            "[MLC]: Receive: (0x" + tostr_hex(message->opcode) + ") Unknown Opcode",
            COLOR_RED
        );
    }
    if (always_log || iter->second.always_log){
        logger.log(
            "[MLC]: Receive: (0x" + tostr_hex(message->opcode) + ") " + iter->second.converter(message),
            color
        );
    }
}


















}
}
