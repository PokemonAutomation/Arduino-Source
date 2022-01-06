/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MessageLogger_H
#define PokemonAutomation_MessageLogger_H

#include <atomic>
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



}
#endif
