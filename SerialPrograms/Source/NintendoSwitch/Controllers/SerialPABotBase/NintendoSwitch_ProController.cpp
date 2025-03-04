/*  Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerTypeStrings.h"
#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch_ProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;







SerialPABotBase_ProController::SerialPABotBase_ProController(
    Logger& logger,
    ControllerType controller_type,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    const ControllerRequirements& requirements
)
    : ProControllerWithScheduler(logger)
    , m_handle(connection)
    , m_serial(connection.botbase())
{
    if (!connection.is_ready()){
        return;
    }

    //  Check compatibility.

    const std::map<ControllerType, std::set<ControllerFeature>>& controllers = connection.supported_controllers();
    auto iter = controllers.find(controller_type);

    std::string missing_feature;
    do{
        if (iter == controllers.end()){
            missing_feature = CONTROLLER_TYPE_STRINGS.get_string(controller_type);
            break;
        }

        missing_feature = requirements.check_compatibility(iter->second);

        if (missing_feature.empty()){
            connection.update_with_capabilities(iter->second);
            return;
        }

    }while (false);

    m_error_string = html_color_text("Missing Feature: " + missing_feature, COLOR_RED);
}
SerialPABotBase_ProController::~SerialPABotBase_ProController(){
    stop();
}




void SerialPABotBase_ProController::cancel_all_commands(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    m_serial->stop_all_commands();
    this->clear_on_next();
}
void SerialPABotBase_ProController::replace_on_next_command(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    m_serial->next_command_interrupt();
    this->clear_on_next();
}


void SerialPABotBase_ProController::wait_for_all(const Cancellable* cancellable){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);

        LoggingSuppressScope scope(m_logging_suppress);
        if (m_logging_suppress.load(std::memory_order_relaxed) == 1){
            m_logger.log("wait_for_all()", COLOR_DARKGREEN);
        }

        if (!is_ready()){
            throw InvalidConnectionStateException();
        }

        this->issue_wait_for_all(cancellable);
    }
    m_serial->wait_for_all_requests(cancellable);
}



void SerialPABotBase_ProController::send_botbase_request(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    m_serial->issue_request(request, cancellable);
}
BotBaseMessage SerialPABotBase_ProController::send_botbase_request_and_wait(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }
    return m_serial->issue_request_and_wait(request, cancellable);
}











}
}
