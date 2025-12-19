/*  SerialPABotBase Messages - Base Protocol Errors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Errors_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Errors_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_ErrorReady : public BotBaseMessageType{
public:
    MessageType_ErrorReady()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_READY",
            PABB_MSG_ERROR_READY,
            0
        )
    {}
};
class MessageType_InvalidMessage : public BotBaseMessageType{
    using Params = pabb_MsgInfoInvalidMessage;
public:
    MessageType_InvalidMessage()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_INVALID_MESSAGE",
            PABB_MSG_ERROR_INVALID_MESSAGE,
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
        ret += ": length = " + std::to_string((unsigned)params.message_length);
        return ret;
    }
};
class MessageType_ChecksumMismatch : public BotBaseMessageType{
    using Params = pabb_MsgInfoChecksumMismatch;
public:
    MessageType_ChecksumMismatch()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_CHECKSUM_MISMATCH",
            PABB_MSG_ERROR_CHECKSUM_MISMATCH,
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
        ret += ": length = " + std::to_string((unsigned)params.message_length);
        return ret;
    }
};
class MessageType_InvalidType : public BotBaseMessageType{
    using Params = pabb_MsgInfoInvalidType;
public:
    MessageType_InvalidType()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_INVALID_TYPE",
            PABB_MSG_ERROR_INVALID_TYPE,
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
        ret += ": type = " + std::to_string((unsigned)params.type);
        return ret;
    }
};
class MessageType_InvalidRequest : public BotBaseMessageType{
    using Params = pabb_MsgInfoInvalidRequest;
public:
    MessageType_InvalidRequest()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_INVALID_REQUEST",
            PABB_MSG_ERROR_INVALID_REQUEST,
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
class MessageType_MissedRequest : public BotBaseMessageType{
    using Params = pabb_MsgInfoMissedRequest;
public:
    MessageType_MissedRequest()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_MISSED_REQUEST",
            PABB_MSG_ERROR_MISSED_REQUEST,
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
class MessageType_CommandDropped : public BotBaseMessageType{
    using Params = pabb_MsgInfoCommandDropped;
public:
    MessageType_CommandDropped()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_COMMAND_DROPPED",
            PABB_MSG_ERROR_COMMAND_DROPPED,
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
class MessageType_ErrorWarning : public BotBaseMessageType{
    using Params = pabb_MsgInfoWARNING;
public:
    MessageType_ErrorWarning()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_WARNING",
            PABB_MSG_ERROR_WARNING,
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
        ret += ": error_code = " + std::to_string(params.error_code);
        return ret;
    }
};
class MessageType_ErrorDisconnected : public BotBaseMessageType{
    using Params = pabb_MsgInfoDisconnected;
public:
    MessageType_ErrorDisconnected()
        : BotBaseMessageType(
            "PABB_MSG_ERROR_DISCONNECTED",
            PABB_MSG_ERROR_DISCONNECTED,
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
        ret += ": error_code = " + std::to_string(params.error_code);
        return ret;
    }
};



}
}
#endif
