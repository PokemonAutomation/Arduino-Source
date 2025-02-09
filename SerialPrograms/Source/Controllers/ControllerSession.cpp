/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ControllerSession.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


void ControllerSession::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg0(m_state_lock);
    m_listeners.add(listener);
}
void ControllerSession::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg0(m_state_lock);
    m_listeners.remove(listener);
}



ControllerSession::~ControllerSession(){
    if (m_connection){
        m_listeners.run_lambda_with_duplicates([&](Listener& listener){
            m_connection->remove_status_listener(listener);
        });
    }
}
ControllerSession::ControllerSession(
    Logger& logger,
    ControllerOption& option,
    const ControllerRequirements& requirements
)
    : m_logger(logger)
    , m_requirements(requirements)
    , m_option(option)
    , m_options_locked(false)
    , m_descriptor(option.current())
    , m_connection(m_descriptor->open_connection(logger))
{
//    cout << "ControllerSession:ControllerSession(): " << m_descriptor->display_name() << endl;
//    cout << "ControllerSession:ControllerSession(): " << m_connection.get() << endl;
    if (m_connection){
        m_connection->add_status_listener(*this);
    }
}


void ControllerSession::get(ControllerOption& option){
    std::lock_guard<std::mutex> lg(m_state_lock);
    option = m_option;
}
void ControllerSession::set(const ControllerOption& option){
    set_device(option.current());
}



bool ControllerSession::ready() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!m_controller){
        return false;
    }
    return m_controller->is_ready();
}
std::shared_ptr<const ControllerDescriptor> ControllerSession::descriptor() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    return m_descriptor;
}
std::string ControllerSession::status_text() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!m_connection){
        return "<font color=\"red\">No controller selected.</font>";
    }
    if (m_controller){
        std::string error_string = m_controller->error_string();
        if (!error_string.empty()){
            return error_string;
        }
    }
    return m_connection->status_text();
}
ControllerConnection& ControllerSession::connection() const{
    if (m_connection){
        return *m_connection;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Connection is null.");
}
AbstractController* ControllerSession::controller() const{
    return m_controller.get();
}




std::string ControllerSession::user_input_blocked() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!m_connection){
        return "<font color=\"red\">No controller selected.</font>";
    }
    if (!m_connection->is_ready()){
        return "<font color=\"red\">Connection is not ready.</font>";
    }
    if (!m_controller->is_ready()){
        return "<font color=\"red\">Controller is not ready.</font>";
    }
    return m_user_input_disallow_reason;
}
void ControllerSession::set_user_input_blocked(std::string disallow_reason){
    std::lock_guard<std::mutex> lg(m_state_lock);
//    cout << "set_user_input_blocked() = " << disallow_reason << endl;
    m_user_input_disallow_reason = std::move(disallow_reason);
}

bool ControllerSession::options_locked() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    return m_options_locked;
}
void ControllerSession::set_options_locked(bool locked){
    bool original_value;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        original_value = m_options_locked;
        m_options_locked = locked;
    }
    if (original_value != locked){
        signal_options_locked(locked);
    }
}


bool ControllerSession::set_device(const std::shared_ptr<const ControllerDescriptor>& device){
//    cout << "set_device() = " << device->display_name() << endl;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        if (*m_descriptor == *device){
            return true;
        }

        if (m_options_locked){
            return false;
        }

        m_controller.reset();
        m_connection.reset();
        m_connection = device->open_connection(m_logger);
        if (m_connection){
            m_connection->add_status_listener(*this);
        }

        m_option.m_current = device;
        m_descriptor = device;
    }
    signal_controller_changed(device);
    signal_status_text_changed(status_text());
    return true;
}



std::string ControllerSession::reset(){
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        if (!m_connection){
            return "No connection set.";
        }
        if (m_options_locked){
            return "Options are locked.";
        }

        m_controller.reset();
        m_connection.reset();
        m_connection = m_descriptor->open_connection(m_logger);
        if (m_connection){
            m_connection->add_status_listener(*this);
        }
    }
    signal_status_text_changed(status_text());
    return "";
}


void ControllerSession::pre_not_ready(){
    m_listeners.run_method_unique(&Listener::pre_not_ready);
}
void ControllerSession::post_ready(const std::map<ControllerType, std::set<ControllerFeature>>& controllers){
#if 1
    if (controllers.empty()){
        return;
    }

    //  REMOVE: TODO: Pick the first controller for now.
    auto iter = controllers.begin();

//    cout << "post_ready()" << endl;

    m_controller.reset();
    m_controller = m_descriptor->make_controller(m_logger, *m_connection, iter->first, m_requirements);
    m_listeners.run_method_unique(&Listener::post_ready, controllers);
    signal_ready_changed(m_controller->is_ready());
#endif
}
void ControllerSession::post_status_text_changed(const std::string& text){
    do{
        if (m_controller == nullptr){
            break;
        }

        std::string error = m_controller->error_string();
        if (!error.empty()){
            m_listeners.run_method_unique(&Listener::post_status_text_changed, error);
            return;
        }
    }while (false);

    m_listeners.run_method_unique(&Listener::post_status_text_changed, text);
}




void ControllerSession::signal_ready_changed(bool ready){
    m_listeners.run_method_unique(&Listener::ready_changed, ready);
}
void ControllerSession::signal_controller_changed(const std::shared_ptr<const ControllerDescriptor>& descriptor){
    m_listeners.run_method_unique(&Listener::controller_changed, descriptor);
}
void ControllerSession::signal_status_text_changed(const std::string& text){
    m_listeners.run_method_unique(&Listener::post_status_text_changed, text);
}
void ControllerSession::signal_options_locked(bool locked){
    m_listeners.run_method_unique(&Listener::options_locked, locked);
}


}
