/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MessageLogger_H
#define PokemonAutomation_MessageLogger_H

#include <atomic>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Containers/CircularBuffer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "MessageSniffer.h"

namespace PokemonAutomation{


class MessageLogger : public Logger, public MessageSniffer{
public:
    MessageLogger(bool log_everything = false)
        : m_log_everything_owner(log_everything)
        , m_log_everything(m_log_everything_owner)
    {}
    MessageLogger(std::atomic<bool>& log_everything)
        : m_log_everything_owner(false)
        , m_log_everything(log_everything)
    {}


    virtual void on_send(const BotBaseMessage& message, bool is_retransmit) override;
    virtual void on_recv(const BotBaseMessage& message) override;

private:
    std::atomic<bool> m_log_everything_owner;
    std::atomic<bool>& m_log_everything;
};



class SerialLogger : public MessageLogger{
public:
    SerialLogger(Logger& logger, bool log_everything);

    virtual void log(const char* msg, Color color = COLOR_DARKGREEN) override;
    virtual void log(const std::string& msg, Color color = COLOR_DARKGREEN) override;

private:
    bool ok_to_log();

private:
    Logger& m_logger;
    SpinLock m_lock;
    size_t m_messages_dropped = 0;
    CircularBuffer<WallClock> m_history;
};




}
#endif
