/*  Serial Port Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "SerialPortSession.h"

namespace PokemonAutomation{



SerialPortSession::~SerialPortSession(){}
SerialPortSession::SerialPortSession(Logger& logger, SerialPortOption& option)
    : m_option(option)
    , m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
    , m_connection(m_logger, option.port(), option.minimum_pabotbase())
{}

QSerialPortInfo SerialPortSession::get() const{
    std::lock_guard<std::mutex> lg(m_lock);
    const QSerialPortInfo* port = m_option.port();
    if (port == nullptr){
        return QSerialPortInfo();
    }else{
        return *port;
    }
}
void SerialPortSession::set(QSerialPortInfo port){
    std::lock_guard<std::mutex> lg(m_lock);
    m_option.set_port(std::move(port));
    m_connection.reset(m_option.port());
}
void SerialPortSession::set(const QSerialPortInfo* port){
    set(port == nullptr ? QSerialPortInfo() : *port);
}

void SerialPortSession::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void SerialPortSession::remove_listener(Listener& listener){
    auto scope_check = m_sanitizer.check_scope();
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
    std::lock_guard<std::mutex> lg(m_lock);
    m_connection.stop();
    for (Listener* listener : m_listeners){
        listener->on_ready(false);
    }
    m_connection.reset(m_option.port());
}





}
