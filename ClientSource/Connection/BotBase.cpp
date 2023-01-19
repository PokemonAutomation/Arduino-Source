/*  Pokemon Automation Bot-Base Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "BotBaseMessage.h"
#include "BotBase.h"

namespace PokemonAutomation{


BotBaseContext::BotBaseContext(BotBase& botbase)
    : m_botbase(botbase)
{}
BotBaseContext::BotBaseContext(CancellableScope& parent, BotBase& botbase)
    : m_botbase(botbase)
{
    attach(parent);
}
BotBaseContext::~BotBaseContext(){
    detach();
}

void BotBaseContext::wait_for_all_requests() const{
    m_lifetime_sanitizer.check_usage();
    m_botbase.wait_for_all_requests(this);
}
void BotBaseContext::cancel_now(){
    m_lifetime_sanitizer.check_usage();
    CancellableScope::cancel(nullptr);
    m_botbase.stop_all_commands();
}
void BotBaseContext::cancel_lazy(){
    m_lifetime_sanitizer.check_usage();
    CancellableScope::cancel(nullptr);
    m_botbase.next_command_interrupt();
}

bool BotBaseContext::cancel(std::exception_ptr exception) noexcept{
    m_lifetime_sanitizer.check_usage();
    if (CancellableScope::cancel(std::move(exception))){
        return true;
    }
    try{
        m_botbase.stop_all_commands();
    }catch (...){}
    return false;
}

bool BotBaseContext::try_issue_request(const BotBaseRequest& request) const{
    m_lifetime_sanitizer.check_usage();
    return m_botbase.try_issue_request(request, this);
}
void BotBaseContext::issue_request(const BotBaseRequest& request) const{
    m_lifetime_sanitizer.check_usage();
    m_botbase.issue_request(request, this);
}

BotBaseMessage BotBaseContext::issue_request_and_wait(const BotBaseRequest& request) const{
    m_lifetime_sanitizer.check_usage();
    return m_botbase.issue_request_and_wait(request, this);
}



}
