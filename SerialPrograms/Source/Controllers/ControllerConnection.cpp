/*  Controller Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ControllerConnection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void ControllerConnection::add_status_listener(StatusListener& listener){
    m_status_listeners.add(listener);
    if (m_ready.load(std::memory_order_acquire)){
        listener.post_connection_ready(*this, supported_controllers());
    }
}
void ControllerConnection::remove_status_listener(StatusListener& listener){
    m_status_listeners.remove(listener);
}

std::string ControllerConnection::status_text() const{
    SpinLockGuard lg(m_status_text_lock);
    return m_status_text;
}


void ControllerConnection::set_status(const std::string& text){
    {
        WriteSpinLock lg(m_status_text_lock);
        m_status_text = text;
    }
//    cout << "ControllerConnection::set_status() = " << text << endl;
    signal_status_text_changed(text);
}
void ControllerConnection::declare_ready(const std::map<ControllerType, std::set<ControllerFeature>>& controllers){
    m_ready.store(true, std::memory_order_release);
    signal_post_ready(controllers);
}


//void ControllerConnection::signal_pre_not_ready(){
//    m_status_listeners.run_method_unique(&StatusListener::pre_connection_not_ready, *this);
//}
void ControllerConnection::signal_post_ready(const std::map<ControllerType, std::set<ControllerFeature>>& controllers){
    m_status_listeners.run_method_unique(&StatusListener::post_connection_ready, *this, controllers);
}
void ControllerConnection::signal_status_text_changed(const std::string& text){
//    cout << "m_status_listeners.size() = " << m_status_listeners.count_unique() << endl;
    m_status_listeners.run_method_unique(&StatusListener::status_text_changed, *this, text);
}
void ControllerConnection::signal_error(const std::string& text){
    m_status_listeners.run_method_unique(&StatusListener::on_error, *this, text);
}




}
