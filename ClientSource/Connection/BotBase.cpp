/*  Pokemon Automation Bot-Base Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "BotBaseMessage.h"
#include "BotBase.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void BotBaseControllerContext::wait_for_all_requests() const{
    m_lifetime_sanitizer.check_usage();
    m_controller.wait_for_all_requests(this);
}
void BotBaseControllerContext::cancel_now(){
    m_lifetime_sanitizer.check_usage();
    CancellableScope::cancel(nullptr);
    m_controller.stop_all_commands();
}
void BotBaseControllerContext::cancel_lazy(){
    m_lifetime_sanitizer.check_usage();
    CancellableScope::cancel(nullptr);
    m_controller.next_command_interrupt();
}

bool BotBaseControllerContext::cancel(std::exception_ptr exception) noexcept{
    m_lifetime_sanitizer.check_usage();
    if (CancellableScope::cancel(std::move(exception))){
        return true;
    }
    try{
        m_controller.stop_all_commands();
    }catch (...){}
    return false;
}

bool BotBaseControllerContext::try_issue_request(const BotBaseRequest& request) const{
    m_lifetime_sanitizer.check_usage();
    return m_controller.try_issue_request(request, this);
}
void BotBaseControllerContext::issue_request(const BotBaseRequest& request) const{
    m_lifetime_sanitizer.check_usage();
    m_controller.issue_request(request, this);
}

BotBaseMessage BotBaseControllerContext::issue_request_and_wait(const BotBaseRequest& request) const{
    m_lifetime_sanitizer.check_usage();
    return m_controller.issue_request_and_wait(request, this);
}



}
