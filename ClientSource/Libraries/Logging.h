/*  Pokemon Automation Bot Base - Client Example
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_Logging_H
#define PokemonAutomation_Logging_H

#include <sstream>
#include <atomic>
#include "ClientSource/Connection/PABotBaseConnection.h"

namespace PokemonAutomation{


extern std::mutex logging_lock;
void log(const std::stringstream& ss);
void log(const std::string& msg);

std::string current_time();


class MessageLogger : public MessageSniffer{
public:
    MessageLogger(bool log_everything = false)
        : m_low_everything_owner(log_everything)
        , m_log_everything(m_low_everything_owner)
    {}
    MessageLogger(std::atomic<bool>& log_everything)
        : m_low_everything_owner(false)
        , m_log_everything(log_everything)
    {}


    virtual void log(std::string msg) override;
    virtual void on_send(const BotBaseMessage& message, bool is_retransmit) override;
    virtual void on_recv(const BotBaseMessage& message) override;

private:
    std::atomic<bool> m_low_everything_owner;
    std::atomic<bool>& m_log_everything;
};




}
#endif
