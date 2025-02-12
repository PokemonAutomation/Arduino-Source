/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Time.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "SysbotBase_Connection.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace SysbotBase{



bool parse_ip_and_port(const std::string& str, QHostAddress& address, int& port){
    //  IPv4
    QStringList parts = QString::fromStdString(str).split(":");
    if (parts.size() != 2){
        return false;
    }
    address = QHostAddress(parts[0]);
    bool ok;
    port = parts[1].toUInt(&ok);
    if (!ok){
        return false;
    }

    //  TODO: IPv6

    return true;
}


SysbotBaseNetwork_Connection::SysbotBaseNetwork_Connection(
    uint64_t sequence_number,
    Logger& logger,
    const std::string& url
)
    : ControllerConnection(sequence_number)
{
    if (!parse_ip_and_port(url, m_address, m_port)){
        set_status(html_color_text("Invalid IP address + port.", COLOR_RED));
        return;
    }

    set_status(html_color_text("Connecting...", COLOR_DARKGREEN));

    m_thread = std::thread(&SysbotBaseNetwork_Connection::thread_body_wrapper, this);
}
SysbotBaseNetwork_Connection::~SysbotBaseNetwork_Connection(){
    m_ready.store(false, std::memory_order_release);
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_socket){
            m_socket->abort();
        }
        m_cv.notify_all();
    }
    if (m_thread.joinable()){
        m_thread.join();
    }
}


std::map<ControllerType, std::set<ControllerFeature>> SysbotBaseNetwork_Connection::supported_controllers() const{
    return {
        {ControllerType::NintendoSwitch_WiredProController, {
            ControllerFeature::NintendoSwitch_ProController
        }},
    };
}



void SysbotBaseNetwork_Connection::write_data(const std::string& data){
    std::lock_guard<std::mutex> lg(m_lock);
    qint64 written = m_socket->write(data.data(), data.size());
    if (written < 0 || written < (qint64)data.size()){
        m_ready.store(false, std::memory_order_release);
        m_stopping.store(true, std::memory_order_release);
        m_cv.notify_all();
    }
}



void SysbotBaseNetwork_Connection::thread_body_wrapper(){
    QTcpSocket socket;
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_socket = &socket;
    }
    try{
        thread_body_internal();
        std::lock_guard<std::mutex> lg(m_lock);
        m_socket = nullptr;
    }catch (...){
        try{
            global_logger_tagged().log("An exception was thrown from the network thread.", COLOR_RED);
            set_status(html_color_text("Error: Unhandled Exception", COLOR_RED));
        }catch (...){}
        std::lock_guard<std::mutex> lg(m_lock);
        m_socket = nullptr;
    }
}
void SysbotBaseNetwork_Connection::thread_body_internal(){
    m_socket->connect(
        m_socket, &QTcpSocket::disconnected,
        m_socket, [this]{
            m_stopping.store(true, std::memory_order_release);
        }
    );

    m_socket->connectToHost(m_address, m_port);

    WallClock start = current_time();
    WallClock deadline = start + std::chrono::seconds(30);
    while (true){
        if (current_time() > deadline){
            set_status(html_color_text("Connection timed out.", COLOR_RED));
            return;
        }
        if (m_stopping.load(std::memory_order_acquire)){
            return;
        }
        if (m_socket->waitForConnected(50)){
            break;
        }
    }

    set_status(html_color_text("Connected!", COLOR_BLUE));

    declare_ready(supported_controllers());

//    m_socket->write("press X\r\n");


}





}
}
