/*  SerialPABotBase Messages - Base Protocol Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Info_H
#define PokemonAutomation_SerialPABotBase_MessageWrappers_BaseProtocol_Info_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "Controllers/SerialPABotBase/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class MessageType_Info_i32 : public BotBaseMessageType{
    using Params = pabb_MsgInfoI32;
public:
    MessageType_Info_i32()
        : BotBaseMessageType(
            "PABB_MSG_INFO_I32",
            PABB_MSG_INFO_I32,
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
        ret += ": tag = " + std::to_string(params.tag);
        ret += ", value = " + std::to_string(params.data);
        return ret;
    }
};
class MessageType_Info_Data : public BotBaseMessageType{
    using Params = pabb_MsgInfoData;
public:
    MessageType_Info_Data()
        : BotBaseMessageType(
            "PABB_MSG_INFO_DATA",
            PABB_MSG_INFO_DATA,
            sizeof(Params)
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
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": tag = " + std::to_string(params.tag);
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
class MessageType_Info_String : public BotBaseMessageType{
public:
    MessageType_Info_String()
        : BotBaseMessageType(
            "PABB_MSG_INFO_STRING",
            PABB_MSG_INFO_STRING,
            0
        )
    {}
    virtual bool is_valid(const std::string& body) const override{
        return true;
    }
    virtual std::string tostr(const std::string& body) const override{
        return BotBaseMessageType::tostr(body) + ": " + body;
    }
};
class MessageType_Info_Label_i32 : public BotBaseMessageType{
    using Params = pabb_MsgInfoI32Label;
public:
    MessageType_Info_Label_i32()
        : BotBaseMessageType(
            "PABB_MSG_INFO_I32_LABEL",
            PABB_MSG_INFO_I32_LABEL,
            sizeof(Params)
        )
    {}
    virtual bool is_valid(const std::string& body) const override{
        return sizeof(Params) <= body.size();
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": " + std::string(body.data() + sizeof(uint32_t), body.size() - sizeof(uint32_t));
        ret += ": " + std::to_string(params.value);
        return ret;
    }
};
class MessageType_Info_Label_h32 : public BotBaseMessageType{
    using Params = pabb_MsgInfoI32Label;
public:
    MessageType_Info_Label_h32()
        : BotBaseMessageType(
            "PABB_MSG_INFO_H32_LABEL",
            PABB_MSG_INFO_H32_LABEL,
            sizeof(Params)
        )
    {}
    virtual bool is_valid(const std::string& body) const override{
        return sizeof(Params) <= body.size();
    }
    virtual std::string tostr(const std::string& body) const override{
        std::string ret = BotBaseMessageType::tostr(body);
        if (!is_valid(body)){
            return ret;
        }
        Params params;
        memcpy(&params, body.data(), sizeof(params));
        ret += ": " + std::string(body.data() + sizeof(uint32_t), body.size() - sizeof(uint32_t));
        ret += ": " + tostr_hex(params.value);
        return ret;
    }
};



}
}
#endif
