/*  PABotBase2 Message Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_MessageHandler_H
#define PokemonAutomation_Controllers_PABotBase2_MessageHandler_H

#include "Common/Compiler.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/PABotBase2/PABotBase2_MessageProtocol.h"

namespace PokemonAutomation{
namespace PABotBase2{



class MessageHandler{
public:
    virtual ~MessageHandler() = default;
    virtual bool should_print() const;
    virtual void assert_is_valid(Logger& logger, const MessageHeader* header) const{}
    virtual std::string tostr(const MessageHeader* header) const = 0;
    virtual void on_recv(Logger& logger, const MessageHeader* header){}

protected:
    void assert_size_eq(Logger& logger, uint16_t bytes, const MessageHeader* header) const;
    void assert_size_min(Logger& logger, uint16_t min_bytes, const MessageHeader* header) const;
};


template <uint8_t opcode, typename MessageType_t>
class FixedLengthMesssageHandler : public MessageHandler{
public:
    using MessageType = MessageType_t;
    static constexpr uint8_t OPCODE = opcode;

    virtual void assert_is_valid(Logger& logger, const MessageHeader* header) const override{
        assert_size_eq(logger, sizeof(MessageType), header);
    }
};


}
}
#endif
