/*  SerialPABotBase Messages - Base Protocol Acks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Acks_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Acks_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_AckCommand : public BotBaseMessageType{
    using Params = pabb_MsgAckCommand;
public:
    MessageType_AckCommand()
        : BotBaseMessageType(
            "PABB_MSG_ACK_COMMAND",
            PABB_MSG_ACK_COMMAND,
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
class MessageType_AckRequest : public BotBaseMessageType{
    using Params = pabb_MsgAckRequest;
public:
    MessageType_AckRequest()
        : BotBaseMessageType(
            "PABB_MSG_ACK_REQUEST",
            PABB_MSG_ACK_REQUEST,
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
class MessageType_AckRequest_i8 : public BotBaseMessageType{
    using Params = pabb_MsgAckRequestI8;
public:
    MessageType_AckRequest_i8()
        : BotBaseMessageType(
            "PABB_MSG_ACK_REQUEST_I8",
            PABB_MSG_ACK_REQUEST_I8,
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
        ret += ", value = " + std::to_string((unsigned)params.data);
        return ret;
    }
};
class MessageType_AckRequest_i16 : public BotBaseMessageType{
    using Params = pabb_MsgAckRequestI16;
public:
    MessageType_AckRequest_i16()
        : BotBaseMessageType(
            "PABB_MSG_ACK_REQUEST_I16",
            PABB_MSG_ACK_REQUEST_I16,
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
        ret += ", value = " + std::to_string(params.data);
        return ret;
    }
};
class MessageType_AckRequest_i32 : public BotBaseMessageType{
    using Params = pabb_MsgAckRequestI32;
public:
    MessageType_AckRequest_i32()
        : BotBaseMessageType(
            "PABB_MSG_ACK_REQUEST_I32",
            PABB_MSG_ACK_REQUEST_I32,
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
        ret += ", value = " + std::to_string(params.data);
        return ret;
    }
};
class MessageType_AckRequest_Data : public BotBaseMessageType{
public:
    MessageType_AckRequest_Data()
        : BotBaseMessageType(
            "PABB_MSG_ACK_REQUEST_DATA",
            PABB_MSG_ACK_REQUEST_DATA,
            sizeof(seqnum_t)
        )
    {}
    virtual bool is_valid(const std::string& body) const override{
        return sizeof(seqnum_t) <= body.size();
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        seqnum_t seqnum;
        memcpy(&seqnum, body.data(), sizeof(seqnum));
        ret += ": seqnum = " + std::to_string(seqnum);
        ret += ", data =";
        static const char HEX_DIGITS[] = "0123456789abcdef";
        for (size_t c = sizeof(seqnum_t); c < body.size(); c++){
            uint8_t byte = body[c];
            ret += " ";
            ret += HEX_DIGITS[(byte >> 4)];
            ret += HEX_DIGITS[byte & 15];
        }
        return ret;
    }
};



}
}
#endif
