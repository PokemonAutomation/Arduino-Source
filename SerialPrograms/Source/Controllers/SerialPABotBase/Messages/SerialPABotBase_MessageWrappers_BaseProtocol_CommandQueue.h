/*  SerialPABotBase Messages - Base Protocol Command Queue
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_CommandQueue_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_CommandQueue_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_CommandFinished : public BotBaseMessageType{
    using Params = pabb_MsgRequestCommandFinished;
public:
    MessageType_CommandFinished()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_COMMAND_FINISHED",
            PABB_MSG_REQUEST_COMMAND_FINISHED,
            sizeof(Params)
        )
    {}
    virtual bool should_print(const std::string& body) const override{
        return GlobalSettings::instance().LOG_EVERYTHING;
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        ret += ", seq_of_original_command = " + std::to_string((unsigned)params.seq_of_original_command);
        ret += ", finish_time = " + std::to_string(params.finish_time);
        return ret;
    }
};



class MessageType_RequestStop : public BotBaseMessageType{
    using Params = pabb_MsgRequestProtocolVersion;
public:
    MessageType_RequestStop()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_STOP",
            PABB_MSG_REQUEST_STOP,
            sizeof(Params)
        )
    {}
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        return ret;
    }
};
class DeviceRequest_request_stop : public BotBaseRequest{
public:
    pabb_MsgRequestProtocolVersion params{};
    DeviceRequest_request_stop()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_STOP, params);
    }
};



class MessageType_NextCommandInterrupt : public BotBaseMessageType{
    using Params = pabb_MsgRequestNextCmdInterrupt;
public:
    MessageType_NextCommandInterrupt()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT",
            PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT,
            sizeof(Params)
        )
    {}
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": seqnum = " + std::to_string(params.seqnum);
        return ret;
    }
};
class DeviceRequest_next_command_interrupt : public BotBaseRequest{
public:
    pabb_MsgRequestNextCmdInterrupt params{};
    DeviceRequest_next_command_interrupt()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT, params);
    }
};



}
}
#endif
