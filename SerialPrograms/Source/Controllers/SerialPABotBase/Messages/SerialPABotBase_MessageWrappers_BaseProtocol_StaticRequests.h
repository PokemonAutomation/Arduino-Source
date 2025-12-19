/*  SerialPABotBase Messages - Base Protocol Static Requests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_StaticRequests_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_StaticRequests_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_SeqnumReset : public BotBaseMessageType{
    using Params = pabb_MsgInfoSeqnumReset;
public:
    MessageType_SeqnumReset()
        : BotBaseMessageType(
            "PABB_MSG_SEQNUM_RESET",
            PABB_MSG_SEQNUM_RESET,
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
class DeviceRequest_seqnum_reset : public BotBaseRequest{
public:
    pabb_MsgInfoSeqnumReset params{};
    DeviceRequest_seqnum_reset()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_SEQNUM_RESET, params);
    }
};



class MessageType_ProtocolVersion : public BotBaseMessageType{
    using Params = pabb_MsgRequestProtocolVersion;
public:
    MessageType_ProtocolVersion()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_PROTOCOL_VERSION",
            PABB_MSG_REQUEST_PROTOCOL_VERSION,
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
class DeviceRequest_protocol_version : public BotBaseRequest{
public:
    pabb_MsgRequestProtocolVersion params{};
    DeviceRequest_protocol_version()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROTOCOL_VERSION, params);
    }
};



class MessageType_ProgramVersion : public BotBaseMessageType{
    using Params = pabb_MsgRequestProgramVersion;
public:
    MessageType_ProgramVersion()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_PROGRAM_VERSION",
            PABB_MSG_REQUEST_PROGRAM_VERSION,
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
class DeviceRequest_program_version : public BotBaseRequest{
public:
    pabb_MsgRequestProgramVersion params{};
    DeviceRequest_program_version()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROGRAM_VERSION, params);
    }
};



class MessageType_ProgramID : public BotBaseMessageType{
    using Params = pabb_MsgRequestProgramID;
public:
    MessageType_ProgramID()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_PROGRAM_ID",
            PABB_MSG_REQUEST_PROGRAM_ID,
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
class DeviceRequest_program_id : public BotBaseRequest{
public:
    pabb_MsgRequestProgramID params{};
    DeviceRequest_program_id()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROGRAM_ID, params);
    }
};



class MessageType_ProgramName : public BotBaseMessageType{
    using Params = pabb_MsgRequestProgramName;

public:
    MessageType_ProgramName()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_PROGRAM_NAME",
            PABB_MSG_REQUEST_PROGRAM_NAME,
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
class DeviceRequest_program_name : public BotBaseRequest{
public:
    pabb_MsgRequestProgramName params{};
    DeviceRequest_program_name()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_PROGRAM_NAME, params);
    }
};



class MessageType_ControllerList : public BotBaseMessageType{
    using Params = pabb_MsgRequestControllerList;
public:
    MessageType_ControllerList()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_CONTROLLER_LIST",
            PABB_MSG_REQUEST_CONTROLLER_LIST,
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
class DeviceRequest_controller_list : public BotBaseRequest{
public:
    pabb_MsgRequestControllerList params{};
    DeviceRequest_controller_list()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_CONTROLLER_LIST, params);
    }
};



class MessageType_QueueSize : public BotBaseMessageType{
    using Params = pabb_MsgRequestQueueSize;

public:
    MessageType_QueueSize()
        : BotBaseMessageType(
            "PABB_MSG_REQUEST_QUEUE_SIZE",
            PABB_MSG_REQUEST_QUEUE_SIZE,
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
class DeviceRequest_queue_size : public BotBaseRequest{
public:
    pabb_MsgRequestQueueSize params{};
    DeviceRequest_queue_size()
        : BotBaseRequest(false)
    {}
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_REQUEST_QUEUE_SIZE, params);
    }
};



}
}
#endif
