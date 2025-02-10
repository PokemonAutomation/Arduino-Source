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
    std::lock_guard<std::mutex> lg(m_state_lock);
    m_listeners.add(listener);
//    if (m_connection && m_connection->is_ready()){
        signal_controller_changed(m_option.m_controller_type, m_available_controllers);
//    }
}
void ControllerSession::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_state_lock);
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
    , m_descriptor(option.descriptor())
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
    set_device(option.descriptor());
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
ControllerType ControllerSession::controller_type() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    return m_option.m_controller_type;
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

        m_option.m_descriptor = device;
        m_descriptor = device;
    }
    signal_descriptor_changed(device);
    signal_status_text_changed(status_text());
    return true;
}
bool ControllerSession::set_controller(ControllerType controller_type){
//    cout << "set_controller()" << endl;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        if (m_option.m_controller_type == controller_type){
            return true;
        }
        if (m_options_locked){
            return false;
        }
        m_option.m_controller_type = controller_type;

        m_controller.reset();
        m_connection.reset();
        m_connection = m_descriptor->open_connection(m_logger);
        if (m_connection){
            m_connection->add_status_listener(*this);
        }
    }
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
    std::vector<ControllerType> available_controllers;

    if (controllers.size() > 1){
        available_controllers.emplace_back(ControllerType::None);
    }

    ControllerType selected_controller = ControllerType::None;
    bool ready = false;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);

        for (const auto& item : controllers){
            available_controllers.emplace_back(item.first);
        }
        m_available_controllers = available_controllers;
        m_controller.reset();

        if (controllers.empty()){
            return;
        }

        auto iter = controllers.begin();

        if (controllers.size() == 1){
            //  Only one controller available. Force the option to it.
            selected_controller = iter->first;
        }else{
            //  Keep the current controller only if it exists.
            iter = controllers.find(m_option.m_controller_type);
            if (iter != controllers.end()){
                selected_controller = m_option.m_controller_type;
            }
        }

//        cout << "post_ready()" << endl;

        if (selected_controller != ControllerType::None){
            m_controller = m_descriptor->make_controller(m_logger, *m_connection, selected_controller, m_requirements);
        }
        m_option.m_controller_type = selected_controller;

        ready = m_controller && m_controller->is_ready();
        signal_controller_changed(selected_controller, available_controllers);
    }
    if (ready){
        m_listeners.run_method_unique(&Listener::post_ready, controllers);
        signal_ready_changed(m_controller->is_ready());
    }
}
void ControllerSession::post_status_text_changed(const std::string& text){
    std::lock_guard<std::mutex> lg(m_state_lock);

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
void ControllerSession::signal_descriptor_changed(
    const std::shared_ptr<const ControllerDescriptor>& descriptor
){
    m_listeners.run_method_unique(&Listener::descriptor_changed, descriptor);
}
void ControllerSession::signal_controller_changed(
    ControllerType controller_type,
    const std::vector<ControllerType>& available_controllers
){
    m_listeners.run_method_unique(&Listener::controller_changed, controller_type, available_controllers);
}
void ControllerSession::signal_status_text_changed(const std::string& text){
    m_listeners.run_method_unique(&Listener::post_status_text_changed, text);
}
void ControllerSession::signal_options_locked(bool locked){
    m_listeners.run_method_unique(&Listener::options_locked, locked);
}


}
