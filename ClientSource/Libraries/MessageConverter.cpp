/*  Message Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <set>
#include <map>
#include <sstream>
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "MessageConverter.h"

namespace PokemonAutomation{


std::map<uint8_t, MessageConverter>& converter_map(){
    static std::map<uint8_t, MessageConverter> converters;
    return converters;
}
void register_message_converter(uint8_t type, MessageConverter converter){
    std::map<uint8_t, MessageConverter>& converters = converter_map();
    auto iter = converters.find(type);
    if (iter != converters.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate message type.");
    }
    converters[type] = converter;
}


int register_message_converters_framework_errors(){
    register_message_converter(
        PABB_MSG_ERROR_READY,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_READY - ";
            if (body.size() != 0){ ss << "(invalid size)" << std::endl; return ss.str(); }
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_INVALID_MESSAGE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_INVALID_MESSAGE - ";
            if (body.size() != sizeof(pabb_MsgInfoInvalidMessage)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoInvalidMessage*)body.c_str();
            ss << "length = " << (unsigned)params->message_length;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_CHECKSUM_MISMATCH,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_CHECKSUM_MISMATCH - ";
            if (body.size() != sizeof(pabb_MsgInfoChecksumMismatch)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoChecksumMismatch*)body.c_str();
            ss << "length = " << (unsigned)params->message_length;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_INVALID_TYPE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_INVALID_TYPE - ";
            if (body.size() != sizeof(pabb_MsgInfoInvalidType)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoInvalidType*)body.c_str();
            ss << "type = " << (unsigned)params->type;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_INVALID_REQUEST,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_INVALID_REQUEST - ";
            if (body.size() != sizeof(pabb_MsgInfoInvalidRequest)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoInvalidRequest*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_MISSED_REQUEST,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_MISSED_REQUEST - ";
            if (body.size() != sizeof(pabb_MsgInfoMissedRequest)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoMissedRequest*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_COMMAND_DROPPED,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_COMMAND_DROPPED - ";
            if (body.size() != sizeof(pabb_MsgInfoCommandDropped)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoCommandDropped*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ERROR_WARNING,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ERROR_WARNING - ";
            if (body.size() != sizeof(pabb_MsgInfoWARNING)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoWARNING*)body.c_str();
            ss << "error code = " << (unsigned)params->error_code;
            switch (params->error_code){
            case 1:
                ss << " (Device was slow to respond to USB request.)";
                break;
            }
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_framework_acks(){
    register_message_converter(
        PABB_MSG_ACK_COMMAND,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ACK_COMMAND - ";
            if (body.size() != sizeof(pabb_MsgAckCommand)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckCommand*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ACK_REQUEST - ";
            if (body.size() != sizeof(pabb_MsgAckRequest)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequest*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST_I8,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ACK_REQUEST_I8 - ";
            if (body.size() != sizeof(pabb_MsgAckRequestI8)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequestI8*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", message = " << (unsigned)params->data;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST_I16,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ACK_REQUEST_I16 - ";
            if (body.size() != sizeof(pabb_MsgAckRequestI16)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequestI16*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", message = " << (unsigned)params->data;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ACK_REQUEST_I32,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ACK_REQUEST_I32 - ";
            if (body.size() != sizeof(pabb_MsgAckRequestI32)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgAckRequestI32*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", message = " << params->data;
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_framework_requests(){
    register_message_converter(
        PABB_MSG_SEQNUM_RESET,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_SEQNUM_RESET - ";
            if (body.size() != sizeof(pabb_MsgInfoSeqnumReset)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoSeqnumReset*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_PROTOCOL_VERSION,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_PROTOCOL_VERSION - ";
            if (body.size() != sizeof(pabb_MsgRequestProtocolVersion)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestProtocolVersion*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_PROGRAM_VERSION,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_PROGRAM_VERSION - ";
            if (body.size() != sizeof(pabb_MsgRequestProgramVersion)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestProgramVersion*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_PROGRAM_ID,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_PROGRAM_ID - ";
            if (body.size() != sizeof(pabb_MsgRequestProgramID)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestProgramID*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_CLOCK,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_CLOCK - ";
            if (body.size() != sizeof(pabb_system_clock)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_system_clock*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_COMMAND_FINISHED,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_COMMAND_FINISHED - ";
            if (body.size() != sizeof(pabb_MsgRequestCommandFinished)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestCommandFinished*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", seq_of_original_command = " << (unsigned)params->seq_of_original_command;
            ss << ", finish_time = " << params->finish_time;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_STOP,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_STOP - ";
            if (body.size() != sizeof(pabb_MsgRequestStop)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestStop*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT - ";
            if (body.size() != sizeof(pabb_MsgRequestStop)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestStop*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_QUEUE_SIZE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_QUEUE_SIZE - ";
            if (body.size() != sizeof(pabb_MsgRequestQueueSize)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgRequestQueueSize*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_END_PROGRAM_CALLBACK,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "end_program_callback() - ";
            if (body.size() != sizeof(pabb_end_program_callback)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_end_program_callback*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SET_LED_STATE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_COMMAND_SET_LED_STATE - ";
            if (body.size() != sizeof(pabb_MsgCommandSetLeds)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgCommandSetLeds*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", on = " << params->on;
            return ss.str();
        }
    );
    return 0;
}
int register_message_converters_custom_info(){
    register_message_converter(
        PABB_MSG_INFO_I32,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_INFO_I32 - ";
            if (body.size() != sizeof(pabb_MsgInfoI32)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfoI32*)body.c_str();
            ss << "tag = " << (unsigned)params->tag;
            ss << ", data = " << params->data;
            return ss.str();
        }
    );
    return 0;
}


int init_MessageLogger =
    register_message_converters_framework_errors() +
    register_message_converters_framework_acks() +
    register_message_converters_framework_requests() +
    register_message_converters_custom_info();


std::string message_to_string(const BotBaseMessage& message){
    const std::map<uint8_t, MessageConverter>& converters = converter_map();
    auto iter = converters.find(message.type);
    if (iter == converters.end()){
        std::ostringstream ss;
        ss << "Unknown Message Type " << (unsigned)message.type << ": length = " << message.body.size();
        return ss.str();
    }
    return iter->second(message.body);
}





}

