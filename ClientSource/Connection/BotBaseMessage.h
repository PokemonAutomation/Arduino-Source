/*  Bot-Base Message
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BotBaseMessage_H
#define PokemonAutomation_BotBaseMessage_H

#include <stdint.h>
#include <cstring>
#include <string>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/LifetimeSanitizer.h"

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
                "Received incorrect response type: " + std::to_string(type)
            );
        }
        if (body.size() != sizeof(MessageBody)){
            throw SerialProtocolException(
                logger, PA_CURRENT_FUNCTION,
                "Received incorrect response size: " + std::to_string(body.size())
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






}
#endif
