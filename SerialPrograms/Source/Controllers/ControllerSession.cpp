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
//        signal_controller_changed(m_option.m_controller_type, m_available_controllers);
//    }
}
void ControllerSession::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_state_lock);
    m_listeners.remove(listener);
}



ControllerSession::~ControllerSession(){
    std::unique_ptr<AbstractController> controller;
    std::unique_ptr<ControllerConnection> connection;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        controller = std::move(m_controller);
        connection = std::move(m_connection);
    }
    controller.reset();
    connection.reset();
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
    , m_connection_is_shutting_down(false)
    , m_descriptor(option.descriptor())
    , m_connection(m_descriptor->open_connection(logger))
{
    if (!m_connection){
        return;
    }

    //  Add listener first so we don't miss the ready signal.
    m_connection->add_status_listener(*this);
    try{
        //  If we already missed it, run it ourselves.
        if (m_connection->is_ready()){
            ControllerSession::post_connection_ready(
                *m_connection, m_connection->supported_controllers()
            );
        }
    }catch (...){
        m_connection->remove_status_listener(*this);
        throw;
    }
}


std::vector<ControllerType> ControllerSession::available_controllers() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    return m_available_controllers;
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


void ControllerSession::make_controller(){
    //  Must be called under "m_reset_lock".

    bool ready = false;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        m_connection = m_descriptor->open_connection(m_logger);
        if (m_connection){
            m_connection->add_status_listener(*this);
            ready = m_connection->is_ready();
        }
    }

    //  If we already missed it, run it ourselves.
    if (ready){
        ControllerSession::post_connection_ready(
            *m_connection, m_connection->supported_controllers()
        );
    }
}



bool ControllerSession::set_device(const std::shared_ptr<const ControllerDescriptor>& device){
//    cout << "set_device() = " << device->display_name() << endl;
    {
        std::lock_guard<std::mutex> lg0(m_reset_lock);

        //  Destroy the current connection+controller.
        std::unique_ptr<AbstractController> controller;
        std::unique_ptr<ControllerConnection> connection;
        {
            std::lock_guard<std::mutex> lg1(m_state_lock);
            if (m_options_locked){
                return false;
            }
            if (*m_descriptor == *device){
                return true;
            }

            //  Move these out to indicate that we should no longer access them.
            controller = std::move(m_controller);
            connection = std::move(m_connection);

//            cout << "setting..." << endl;
            m_option.set_descriptor(device);
            m_descriptor = device;
        }

        //  With the lock released, it is now safe to destroy them.
        //  We cannot destroy these under (m_state_lock) due to their asynchronous
        //  callbacks into this class which will also acquire the same lock.
        controller.reset();
        connection.reset();

        make_controller();
    }
    signal_descriptor_changed(device);
    signal_status_text_changed(status_text());
    return true;
}
bool ControllerSession::set_controller(ControllerType controller_type){
//    cout << "set_controller()" << endl;
    std::shared_ptr<const ControllerDescriptor> device;
    {
        std::lock_guard<std::mutex> lg0(m_reset_lock);

        //  Destroy the current connection+controller.
        std::unique_ptr<AbstractController> controller;
        std::unique_ptr<ControllerConnection> connection;
        {
            std::lock_guard<std::mutex> lg1(m_state_lock);
            if (m_options_locked){
                return false;
            }
            if (m_option.m_controller_type == controller_type){
                return true;
            }

            //  Move these out to indicate that we should no longer access them.
            controller = std::move(m_controller);
            connection = std::move(m_connection);

            m_option.m_controller_type = controller_type;
        }

        //  With the lock released, it is now safe to destroy them.
        //  We cannot destroy these under (m_state_lock) due to their asynchronous
        //  callbacks into this class which will also acquire the same lock.
        controller.reset();
        connection.reset();

        make_controller();
        device = m_descriptor;
    }
    signal_descriptor_changed(device);
    signal_status_text_changed(status_text());
    return true;
}



std::string ControllerSession::reset(){
    {
        std::lock_guard<std::mutex> lg0(m_reset_lock);

        //  Destroy the current connection+controller.
        std::unique_ptr<AbstractController> controller;
        std::unique_ptr<ControllerConnection> connection;
        {
            std::lock_guard<std::mutex> lg1(m_state_lock);
            if (m_options_locked){
                return "Options are locked.";
            }
            if (!m_connection){
                return "No connection set.";
            }

            //  Move these out to indicate that we should no longer access them.
            controller = std::move(m_controller);
            connection = std::move(m_connection);
        }

        //  With the lock released, it is now safe to destroy them.
        //  We cannot destroy these under (m_state_lock) due to their asynchronous
        //  callbacks into this class which will also acquire the same lock.
        controller.reset();
        connection.reset();

        make_controller();
    }
    signal_status_text_changed(status_text());
    return "";
}


//void ControllerSession::pre_connection_not_ready(ControllerConnection& connection){
//    m_listeners.run_method_unique(&Listener::pre_connection_not_ready, connection);
//}
void ControllerSession::post_connection_ready(
    ControllerConnection& connection,
    const std::map<ControllerType, std::set<ControllerFeature>>& controllers
){
    if (controllers.empty()){
        return;
    }

//    cout << "sleeping" << endl;
//    Sleep(10000);


    std::vector<ControllerType> available_controllers;
    ControllerType selected_controller = ControllerType::None;
    bool ready;
    {
        std::lock_guard<std::mutex> lg(m_state_lock);

        //  Connection has already been closed.
        if (m_connection == nullptr){
            return;
        }

        //  Controller already constructed.
        if (m_controller){
            return;
        }

        //  We only show the "none" option when there are multiple controllers
        //  to choose from.
        if (controllers.size() > 1){
            available_controllers.emplace_back(ControllerType::None);
        }
        for (const auto& item : controllers){
            available_controllers.emplace_back(item.first);
        }

        //  Copy the list. One will be moved into "m_available_controllers".
        //  The other will be stored locally for the callbacks.
        m_available_controllers = available_controllers;


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

        //  Construct the controller.
        if (selected_controller != ControllerType::None){
            m_controller = m_descriptor->make_controller(
                m_logger,
                *m_connection,
                selected_controller,
                m_requirements
            );
        }

        //  Commit all changes.
        m_option.m_controller_type = selected_controller;
        ready = m_controller && m_controller->is_ready();

        WriteSpinLock lg1(m_message_lock);
        m_controller_error = m_controller->error_string();
    }

    signal_controller_changed(selected_controller, available_controllers);
    signal_ready_changed(ready);
    signal_status_text_changed(status_text());
}
void ControllerSession::status_text_changed(
    ControllerConnection& connection,
    const std::string& text
){
//    cout << "ControllerSession::status_text_changed() = " << text << endl;
    std::string controller_error;
    {
        WriteSpinLock lg(m_message_lock);
        controller_error = m_controller_error;
    }
//    cout << "ControllerSession::status_text_changed(): controller_error = " << controller_error << endl;
    if (controller_error.empty()){
        m_listeners.run_method_unique(&Listener::post_status_text_changed, text);
    }else{
        m_listeners.run_method_unique(&Listener::post_status_text_changed, controller_error);
    }
}
void ControllerSession::on_error(
    ControllerConnection& connection,
    const std::string& text
){
    WriteSpinLock lg(m_message_lock);
    m_controller_error = text;
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
//    cout << text << endl;
    m_listeners.run_method_unique(&Listener::post_status_text_changed, text);
}
void ControllerSession::signal_options_locked(bool locked){
    m_listeners.run_method_unique(&Listener::options_locked, locked);
}


}
