/*  Controller Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ControllerTypeStrings.h"
#include "ControllerSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


void ControllerSession::add_listener(Listener& listener){
//    WriteSpinLock lg(m_state_lock);
    m_listeners.add(listener);
//    if (m_connection && m_connection->is_ready()){
//        signal_controller_changed(m_option.m_controller_type, m_available_controllers);
//    }
}
void ControllerSession::remove_listener(Listener& listener){
//    WriteSpinLock lg(m_state_lock);
    m_listeners.remove(listener);
}



ControllerSession::~ControllerSession(){
    std::unique_ptr<AbstractController> controller;
    std::unique_ptr<ControllerConnection> connection;
    {
        WriteSpinLock lg(m_state_lock);
        controller = std::move(m_controller);
        connection = std::move(m_connection);
    }
    controller.reset();
    connection.reset();
}
ControllerSession::ControllerSession(
    Logger& logger,
    ControllerOption& option
)
    : m_logger(logger)
    , m_option(option)
    , m_options_locked(false)
    , m_desired_controller(ControllerType::None)
    , m_next_reset_mode(ControllerResetMode::DO_NOT_RESET)
    , m_descriptor(option.descriptor())
    , m_connection(m_descriptor->open_connection(logger, false))
{
    if (!m_connection){
        return;
    }

    //  Add listener first so we don't miss the ready signal.
    m_connection->add_status_listener(*this);
    try{
        //  If we already missed it, run it ourselves.
        if (m_connection->is_ready()){
//            cout << "ControllerSession::ControllerSession() - early ready" << endl;
            m_desired_controller = m_connection->current_controller();
            ControllerSession::post_connection_ready(*m_connection);
        }
    }catch (...){
        m_connection->remove_status_listener(*this);
        throw;
    }
}


std::vector<ControllerType> ControllerSession::available_controllers() const{
    ReadSpinLock lg(m_state_lock);
    if (!m_connection || !m_connection->is_ready()){
        return {};
    }
    return m_connection->controller_list();
}

void ControllerSession::get(ControllerOption& option){
    ReadSpinLock lg(m_state_lock);
    option = m_option;
}
void ControllerSession::set(const ControllerOption& option){
    set_device(option.descriptor());
}



bool ControllerSession::ready() const{
    ReadSpinLock lg(m_state_lock);
    if (!m_controller){
        return false;
    }
    return m_controller->is_ready();
}
std::shared_ptr<const ControllerDescriptor> ControllerSession::descriptor() const{
    ReadSpinLock lg(m_state_lock);
    return m_descriptor;
}
ControllerType ControllerSession::controller_type() const{
    ReadSpinLock lg(m_state_lock);
    if (!m_connection){
        return ControllerType::None;
    }
    return m_connection->current_controller();
}
std::string ControllerSession::status_text() const{
    ReadSpinLock lg(m_state_lock);
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
AbstractController* ControllerSession::controller() const{
    return m_controller.get();
}




std::string ControllerSession::user_input_blocked() const{
    ReadSpinLock lg(m_state_lock);
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
    ReadSpinLock lg(m_state_lock);
//    cout << "set_user_input_blocked() = " << disallow_reason << endl;
    m_user_input_disallow_reason = std::move(disallow_reason);
}

bool ControllerSession::options_locked() const{
    ReadSpinLock lg(m_state_lock);
    return m_options_locked;
}
void ControllerSession::set_options_locked(bool locked){
    bool original_value;
    {
        WriteSpinLock lg(m_state_lock);
        original_value = m_options_locked;
        m_options_locked = locked;
    }
    if (original_value != locked){
        signal_options_locked(locked);
    }
}


void ControllerSession::make_controller(
    std::optional<ControllerType> change_controller,
    bool clear_settings
){
//    cout << "ControllerSession::make_controller()" << endl;

    //  Must be called under "m_reset_lock".

    bool ready = false;
    {
        WriteSpinLock lg(m_state_lock);
//        cout << "clear_settings = " << clear_settings << endl;
        m_connection = m_descriptor->open_connection(m_logger, change_controller == ControllerType::None);
        if (m_connection){
            m_connection->add_status_listener(*this);
            ready = m_connection->is_ready();
        }
    }

//    m_desired_controller = m_connection->current_controller();
    m_next_reset_mode = ControllerResetMode::DO_NOT_RESET;
    if (change_controller.has_value()){
        m_desired_controller = change_controller.value();
        m_next_reset_mode = ControllerResetMode::SIMPLE_RESET;
    }
    if (clear_settings){
        m_next_reset_mode = ControllerResetMode::RESET_AND_CLEAR_STATE;
    }

    //  If we already missed it, run it ourselves.
    if (ready){
        ControllerSession::post_connection_ready(*m_connection);
    }

    signal_descriptor_changed(m_descriptor);
}



bool ControllerSession::set_device(const std::shared_ptr<const ControllerDescriptor>& device){
//    cout << "ControllerSession::set_device() = " << device->display_name() << endl;
    {
        std::lock_guard<std::mutex> lg0(m_reset_lock);

        //  Destroy the current connection+controller.
        std::unique_ptr<AbstractController> controller;
        std::unique_ptr<ControllerConnection> connection;
        {
            WriteSpinLock lg1(m_state_lock);
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

        make_controller({}, false);
    }
    signal_descriptor_changed(device);
    signal_status_text_changed(status_text());
    return true;
}
bool ControllerSession::set_controller(ControllerType controller_type){
//    cout << "ControllerSession::set_controller()" << endl;
    std::shared_ptr<const ControllerDescriptor> device;
    {
        std::lock_guard<std::mutex> lg0(m_reset_lock);

        //  Destroy the current connection+controller.
        std::unique_ptr<AbstractController> controller;
        std::unique_ptr<ControllerConnection> connection;
        {
            WriteSpinLock lg1(m_state_lock);
            if (m_options_locked){
                return false;
            }
            if (m_connection && m_connection->current_controller() == controller_type){
                return true;
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

        make_controller(controller_type, false);
        device = m_descriptor;
    }
    signal_descriptor_changed(device);
    signal_status_text_changed(status_text());
    return true;
}



std::string ControllerSession::reset(bool clear_settings){
//    cout << "ControllerSession::reset()" << endl;

    {
        std::lock_guard<std::mutex> lg0(m_reset_lock);

//        std::optional<ControllerType> change_controller;

        //  Destroy the current connection+controller.
        std::unique_ptr<AbstractController> controller;
        std::unique_ptr<ControllerConnection> connection;
        {
            WriteSpinLock lg1(m_state_lock);
            if (m_options_locked){
                return "Options are locked.";
            }
//            if (!m_connection){
//                cout << "ControllerSession::reset() - early return" << endl;
//                return "No connection set.";
//            }

//            cout << "Checking readiness... " << (int)m_desired_controller << endl;
            if (m_connection && m_connection->is_ready()){
                m_desired_controller = m_connection->current_controller();
//                cout << "Ready! - " << (int)m_desired_controller << endl;
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

        make_controller(m_desired_controller, clear_settings);
    }
    signal_status_text_changed(status_text());
    return "";
}


//void ControllerSession::pre_connection_not_ready(ControllerConnection& connection){
//    m_listeners.run_method_unique(&Listener::pre_connection_not_ready, connection);
//}
void ControllerSession::post_connection_ready(ControllerConnection& connection){
//    cout << "ControllerSession::post_connection_ready()" << endl;

//    ControllerType current_controller = mode_status.current_controller;

//    cout << "sleeping" << endl;
//    Sleep(10000);

    std::vector<ControllerType> supported_controllers;
    ControllerType current_controller = ControllerType::None;

    ControllerType desired_controller;
    ControllerResetMode reset_mode;

    std::unique_ptr<AbstractController> controller;

    bool ready;
    {
        ReadSpinLock lg(m_state_lock);

        //  Connection has already been closed.
        if (m_connection == nullptr){
            return;
        }

        //  Controller already constructed.
        if (m_controller){
            return;
        }

        desired_controller = m_desired_controller;
        if (m_next_reset_mode == ControllerResetMode::DO_NOT_RESET){
            desired_controller = m_connection->current_controller();
        }

        reset_mode = m_next_reset_mode;

        supported_controllers = m_connection->controller_list();
        current_controller = m_connection->current_controller();
    }
    signal_controller_changed(current_controller, supported_controllers);

    //  Construct the controller.
    if (desired_controller != ControllerType::None){
        controller = m_descriptor->make_controller(
            m_logger,
            connection,
            desired_controller,
            reset_mode
        );
    }

    {
        WriteSpinLock lg(m_state_lock);

        //  Connection has been wiped. ABA is safe here.
        if (m_connection.get() != &connection){
            return;
        }

        m_controller = std::move(controller);
        m_desired_controller = desired_controller;
        m_next_reset_mode = ControllerResetMode::DO_NOT_RESET;

        supported_controllers = m_connection->controller_list();
        current_controller = m_connection->current_controller();

        //  Commit all changes.
//        cout << "current_controller = " << CONTROLLER_TYPE_STRINGS.get_string(current_controller) << endl;
        ready = m_controller && m_controller->is_ready();
    }
    signal_controller_changed(current_controller, supported_controllers);
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
