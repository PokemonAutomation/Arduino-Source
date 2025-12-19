/*  Bot-Base Message
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_BotBaseMessage_H
#define PokemonAutomation_BotBaseMessage_H

#include <stdint.h>
#include <cstring>
#include <string>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "CommonFramework/GlobalSettingsPanel.h"

namespace PokemonAutomation{


struct BotBaseMessage{
    uint8_t type;
    std::string body;

    LifetimeSanitizer sanitizer;

    BotBaseMessage() = default;
    BotBaseMessage(uint8_t p_type, std::string p_body)
        : type(p_type)
        , body(std::move(p_body))
    {}

    template <typename Params>
    BotBaseMessage(uint8_t p_type, const Params& params)
        : type(p_type)
        , body((char*)&params, sizeof(params))
    {}

    template <uint8_t MessageType, typename MessageBody>
    void convert(Logger& logger, MessageBody& params) const{
        sanitizer.check_usage();
        if (type != MessageType){
            throw SerialProtocolException(
                logger, PA_CURRENT_FUNCTION,
                "Received Incorrect Response Type: Expected = " + std::to_string(MessageType) + ", Actual = " + std::to_string(type)
            );
        }
        if (body.size() != sizeof(MessageBody)){
            throw SerialProtocolException(
                logger, PA_CURRENT_FUNCTION,
                "Received Incorrect Response Size: Expected = " + std::to_string(sizeof(MessageBody)) + ", Actual = " + std::to_string(body.size())
            );
        }
        memcpy(&params, body.c_str(), body.size());
        sanitizer.check_usage();
    }

};

class BotBaseRequest{
public:
    BotBaseRequest(bool is_command)
        : m_is_command(is_command)
    {}
    virtual ~BotBaseRequest() = default;
    virtual BotBaseMessage message() const = 0;

    bool is_command() const{ return m_is_command; }

private:
    bool m_is_command;
};





class BotBaseMessageType{
public:
    template <typename ChildType>
    static const BotBaseMessageType& instance(){
        static const ChildType type;
        return type;
    }

    BotBaseMessageType(const char* name, uint8_t type, uint8_t body_size)
        : m_name(name)
        , m_type(type)
        , m_size(body_size)
    {}

    uint8_t type() const{
        return m_type;
    }

    friend bool operator<(const BotBaseMessageType& x, const BotBaseMessageType& y){
        return x.m_type < y.m_type;
    }


public:
    virtual bool should_print(const std::string& body) const{
        if (GlobalSettings::instance().LOG_EVERYTHING){
            return true;
        }
        if (PABB_MSG_IS_ACK(m_type)){
            return false;
        }
        return true;
    }
    virtual bool is_valid(const std::string& body) const{
        return body.size() == m_size;
    }
    virtual std::string tostr(const std::string& body) const{
        std::string ret;
        ret += "(0x" + tostr_hex(m_type) + ") ";
        ret += m_name;
        if (!is_valid(body)){
            ret += ": (Invalid: Size = " + std::to_string(body.size()) + ")";
        }
        return ret;
    }


private:
    const char* m_name;
    const uint8_t m_type;
    const uint8_t m_size;   //  Excluding header and CRC.
};








}
#endif
