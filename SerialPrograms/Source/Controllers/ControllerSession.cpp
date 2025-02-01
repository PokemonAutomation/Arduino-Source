/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ControllerSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


void ControllerSession::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg0(m_state_lock);
    m_listeners.add(listener);
    if (m_connection){
        m_connection->add_status_listener(listener);
    }
}
void ControllerSession::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg0(m_state_lock);
    m_listeners.remove(listener);
    if (m_connection){
        m_connection->remove_status_listener(listener);
    }
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
    , m_connection(m_descriptor->open(logger, m_requirements))
{
//    cout << "ControllerSession:ControllerSession(): " << m_descriptor->display_name() << endl;
//    cout << "ControllerSession:ControllerSession(): " << m_connection.get() << endl;
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
    if (!m_connection){
        return false;
    }
    return m_connection->ready();
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
    return m_connection->status_text();
}
ControllerConnection& ControllerSession::connection() const{
    if (m_connection){
        return *m_connection;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Connection is null.");
}




std::string ControllerSession::user_input_blocked() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!m_connection){
        return "<font color=\"red\">No controller selected.</font>";
    }
    if (!m_connection->ready()){
        return "<font color=\"red\">Connection is not ready.</font>";
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

        std::unique_ptr<ControllerConnection> connection = device->open(
            m_logger, m_requirements
        );
        if (connection){
            m_listeners.run_lambda_with_duplicates([&](Listener& listener){
                connection->add_status_listener(listener);
            });
        }

        m_option.m_current = device;
        m_descriptor = device;
        m_connection = std::move(connection);
    }
    signal_controller_changed(device);
    signal_status_text_changed(status_text());
    return true;
}



std::string ControllerSession::reset(){
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        if (!m_connection){
            return "No controller set.";
        }
        if (m_options_locked){
            return "Options are locked.";
        }

        m_connection.reset();
        m_connection = m_descriptor->open(m_logger, m_requirements);
        if (m_connection){
            m_listeners.run_lambda_with_duplicates([&](Listener& listener){
                m_connection->add_status_listener(listener);
            });
        }
    }
    signal_status_text_changed(status_text());
    return "";
}



void ControllerSession::signal_ready_changed(bool ready){
    m_listeners.run_method_unique(&Listener::ready_changed, ready);
}
void ControllerSession::signal_controller_changed(const std::shared_ptr<const ControllerDescriptor>& descriptor){
    m_listeners.run_method_unique(&Listener::controller_changed, descriptor);
}
void ControllerSession::signal_status_text_changed(const std::string& text){
    m_listeners.run_method_unique(&Listener::status_text_changed, text);
}
void ControllerSession::signal_options_locked(bool locked){
    m_listeners.run_method_unique(&Listener::options_locked, locked);
}


}
