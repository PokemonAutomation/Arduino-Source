/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "SerialPABotBase_Connection.h"

namespace PokemonAutomation{
namespace SerialPABotBase{




SerialConnection::SerialConnection(
    Logger& logger,
    const SerialDescriptor& descriptor,
    const ControllerRequirements& requirements
)
    : m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
    , m_handle(m_logger, &descriptor.port(), requirements)
{
//    cout << "SerialConnection:SerialConnection" << endl;

    connect(
        &m_handle, &BotBaseHandle::on_not_connected,
        this, [this](std::string error){
//            cout << "BotBaseHandle::on_not_connected: " << error << endl;
            m_status = std::move(error);
            m_ready.store(false, std::memory_order_release);
            update_status_string();
            signal_ready_changed(false);
        }
    );
    connect(
        &m_handle, &BotBaseHandle::on_connecting,
        this, [this](const std::string& port_name){
//            cout << "BotBaseHandle::on_connecting" << endl;
            m_status = "<font color=\"green\">Connecting...</font>";
            m_ready.store(false, std::memory_order_release);
            update_status_string();
            signal_ready_changed(false);
        }
    );
    connect(
        &m_handle, &BotBaseHandle::on_ready,
        this, [this](std::string description){
//            cout << "BotBaseHandle::on_ready: " << description << endl;
            m_status = std::move(description);
            m_ready.store(true, std::memory_order_release);
            update_status_string();
            signal_ready_changed(true);
        }
    );
    connect(
        &m_handle, &BotBaseHandle::on_stopped,
        this, [this](std::string error){
//            cout << "BotBaseHandle::on_stopped: " << error << endl;
            m_status = std::move(error);
            m_ready.store(false, std::memory_order_release);
            update_status_string();
            signal_ready_changed(false);
        }
    );
    connect(
        &m_handle, &BotBaseHandle::uptime_status,
        this, [this](std::string status){
            m_uptime = std::move(status);
            update_status_string();
        }
    );

    m_status_text = m_handle.label();
}

void SerialConnection::update_status_string(){
    std::string str;
    str += m_status;
    if (!str.empty() && !m_uptime.empty()){
        str += "<br>";
    }
    str += m_uptime;
    {
        SpinLockGuard lg(m_status_text_lock);
        m_status_text = str;
    }

//    cout << "status: " << str << endl;

    signal_status_text_changed(str);
}


//const char* SerialConnection::reset(){
//    return m_handle.try_reset();
//}
std::string SerialConnection::stop_pending_commands(){
    const char* error = m_handle.try_stop_commands();
    return error ? error : "";
}
std::string SerialConnection::set_next_command_replace(){
    const char* error = m_handle.try_next_interrupt();
    return error ? error : "";
}
std::string SerialConnection::send_request(const BotBaseRequest& request){
    const char* error = m_handle.try_send_request(request);
    return error ? error : "";
}




}
}
