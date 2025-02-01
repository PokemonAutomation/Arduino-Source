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
}
void ControllerConnection::remove_status_listener(StatusListener& listener){
    m_status_listeners.remove(listener);
}

std::string ControllerConnection::status_text() const{
    SpinLockGuard lg(m_status_text_lock);
    return m_status_text;
}

void ControllerConnection::signal_ready_changed(bool ready){
    m_status_listeners.run_method_unique(&StatusListener::ready_changed, ready);
}
void ControllerConnection::signal_status_text_changed(const std::string& text){
//    cout << "m_status_listeners.size() = " << m_status_listeners.count_unique() << endl;
    m_status_listeners.run_method_unique(&StatusListener::status_text_changed, text);
}




}
