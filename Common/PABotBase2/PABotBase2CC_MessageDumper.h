/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_MessageDumper_H
#define PokemonAutomation_PABotBase2_MessageDumper_H

#include <string>
#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "ReliableConnectionLayer/PABotBase2_PacketProtocol.h"
#include "PABotBase2_MessageProtocol.h"

namespace PokemonAutomation{
namespace PABotBase2{


std::string tostr(const PacketHeader* header);
//std::string tostr(const MessageHeader* header);


//void log_send(Logger& logger, const std::string& message, Color color = COLOR_DARKGREEN);
//void log_send(Logger& logger, const MessageHeader& message, Color color = COLOR_DARKGREEN);
//void log_receive(Logger& logger, const MessageHeader& message, Color color = COLOR_PURPLE);



class MessageLogger{
public:
    MessageLogger();

    void add_message(
        uint8_t opcode,
        bool always_print,
        std::string(*tostr)(const MessageHeader*)
    );

    std::string to_str(const MessageHeader* message);
    void log_send(
        Logger& logger,
        bool always_log,
        const MessageHeader* message,
        Color color = COLOR_DARKGREEN
    );
    void log_recv(
        Logger& logger,
        bool always_log,
        const MessageHeader* message,
        Color color = COLOR_PURPLE
    );


private:
    struct Converter{
        bool always_log;
        std::string(*converter)(const MessageHeader*);
    };
    std::map<uint8_t, Converter> m_converters;
};




}
}
#endif
