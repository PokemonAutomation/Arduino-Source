/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_MessageDumper_H
#define PokemonAutomation_PABotBase2_MessageDumper_H

#include <string>
#include <map>
#include <functional>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "ReliableConnectionLayer/PABotBase2_PacketProtocol.h"
#include "PABotBase2_MessageProtocol.h"

namespace PokemonAutomation{
namespace PABotBase2{


std::string tostr(const PacketHeader* header);



struct MessagePrinter{
    const char* message_name;
    uint16_t min_length;
    uint16_t max_length;
    std::function<bool(const MessageHeader*)> should_log;
    std::function<std::string(const MessageHeader*)> to_str;

    std::string tostr(const MessageHeader* message) const;
};


class MessageLogger{
public:
    MessageLogger();

    void add_message(
        const char* message_name,
        uint8_t opcode,
        uint16_t min_length,
        uint16_t max_length,
        std::function<bool(const MessageHeader*)> should_log,
        std::function<std::string(const MessageHeader*)> to_str
    );
    template <typename MessageType>
    void add_message(
        const char* message_name,
        uint8_t opcode,
        bool always_print,
        std::string(*tostr)(const MessageType*)
    ){
        add_message(
            message_name, opcode,
            sizeof(MessageType), sizeof(MessageType),
            [=](const MessageHeader*){ return always_print; },
            [=](const MessageHeader* header){
                return tostr((const MessageType*)header);
            }
        );
    }
    template <typename MessageType>
    void add_message(
        const char* message_name,
        uint8_t opcode,
        bool(*should_print)(const MessageType*),
        std::string(*tostr)(const MessageType*)
    ){
        add_message(
            message_name, opcode,
            sizeof(MessageType), sizeof(MessageType),
            [=](const MessageHeader* header){
                return should_print((const MessageType*)header);
            },
            [=](const MessageHeader* header){
                return tostr((const MessageType*)header);
            }
        );
    }
    template <typename MessageType>
    void add_message_min_length(
        const char* message_name,
        uint8_t opcode,
        bool always_print,
        std::string(*tostr)(const MessageType*)
    ){
        add_message(
            message_name, opcode,
            sizeof(MessageType), (uint16_t)-1,
            [=](const MessageHeader*){ return always_print; },
            [=](const MessageHeader* header){
                return tostr((const MessageType*)header);
            }
        );
    }
    template <typename MessageType>
    void add_message_min_length(
        const char* message_name,
        uint8_t opcode,
        bool(*should_print)(const MessageType*),
        std::string(*tostr)(const MessageType*)
    ){
        add_message(
            message_name, opcode,
            sizeof(MessageType), (uint16_t)-1,
            [=](const MessageHeader* header){
                return should_print((const MessageType*)header);
            },
            [=](const MessageHeader* header){
                return tostr((const MessageType*)header);
            }
        );
    }

    std::string to_str(const MessageHeader* message) const;
    void log_send(
        Logger& logger,
        bool always_log,
        const MessageHeader* message,
        Color color = COLOR_DARKGREEN
    ) const noexcept;
    void log_recv(
        Logger& logger,
        bool always_log,
        const MessageHeader* message,
        Color color = COLOR_PURPLE
    ) const noexcept;


private:
    mutable Mutex m_lock;
    std::map<uint8_t, MessagePrinter> m_converters;
};




}
}
#endif
