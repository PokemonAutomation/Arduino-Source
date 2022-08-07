/*  Serial Port Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "SerialPortSession.h"

namespace PokemonAutomation{



SerialPortSession::SerialPortSession(Logger& logger, SerialPortOption& option)
    : m_option(option)
    , m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
    , m_connection(m_logger, option.port(), option.minimum_pabotbase())
{}
SerialPortSession::~SerialPortSession(){

}

void SerialPortSession::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void SerialPortSession::remove_listener(Listener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
}
void SerialPortSession::push_ready(bool ready){
    std::lock_guard<std::mutex> lg(m_lock);
    for (Listener* listener : m_listeners){
        listener->on_ready(ready);
    }
}



void SerialPortSession::stop(){
    m_connection.stop();
}
void SerialPortSession::reset(){
    stop();
    push_ready(false);
    m_connection.reset(m_option.port());
}





}
