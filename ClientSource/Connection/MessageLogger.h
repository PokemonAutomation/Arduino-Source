/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MessageLogger_H
#define PokemonAutomation_MessageLogger_H

#include <atomic>
#include "Common/Cpp/AbstractLogger.h"
#include "MessageSniffer.h"

namespace PokemonAutomation{


class MessageLogger : public MessageSniffer{
public:
    MessageLogger(bool log_everything = false)
        : m_log_everything_owner(log_everything)
        , m_log_everything(m_log_everything_owner)
    {}
    MessageLogger(std::atomic<bool>& log_everything)
        : m_log_everything_owner(false)
        , m_log_everything(log_everything)
    {}


//    virtual void log(std::string msg) override;
    virtual void on_send(const BotBaseMessage& message, bool is_retransmit) override;
    virtual void on_recv(const BotBaseMessage& message) override;

private:
    std::atomic<bool> m_log_everything_owner;
    std::atomic<bool>& m_log_everything;
};



class SerialLogger : public Logger, public MessageLogger{
public:
    SerialLogger(Logger& logger, bool log_everything);

    virtual void log(const char* msg, Color color = Color()) override;
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(std::string msg) override;

private:
    Logger& m_logger;
};




}
#endif
