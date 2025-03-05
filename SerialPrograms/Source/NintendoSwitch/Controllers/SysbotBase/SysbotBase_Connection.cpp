/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QEventLoop>
#include "Common/Cpp/Time.h"
//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "SysbotBase_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SysbotBase{

using namespace std::chrono_literals;



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




TcpSysbotBase_Connection::TcpSysbotBase_Connection(
    Logger& logger,
    const std::string& url
)
    : m_logger(logger)
    , m_last_receive(WallClock::min())
{
    QHostAddress address;
    int port;
    if (!parse_ip_and_port(url, address, port)){
        set_status_line0(html_color_text("Invalid IP address + port.", COLOR_RED));
        return;
    }

    set_status_line0(html_color_text("Connecting...", COLOR_DARKGREEN));

    m_connecting_message =
        "Connecting To: " + address.toString().toStdString() +
        " - Port: " + std::to_string(port);
    m_logger.log(m_connecting_message);

    //  Attach ourselves as listener first since it might immediately return as
    //  connected.
    m_socket.add_listener(*this);

    try{
        m_socket.connect(address.toString().toStdString(), port);
    }catch (...){
        m_socket.remove_listener(*this);
        throw;
    }
}

TcpSysbotBase_Connection::~TcpSysbotBase_Connection(){
    try{
        write_data("detachController\n");
    }catch (...){}
    m_socket.remove_listener(*this);
    m_socket.close();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }
    if (m_thread.joinable()){
        m_thread.join();
    }
}


ControllerModeStatus TcpSysbotBase_Connection::controller_mode_status() const{
    return {
        ControllerType::NintendoSwitch_WiredProController,
        {
            {ControllerType::NintendoSwitch_WiredProController, {
                ControllerFeature::NintendoSwitch_ProController
            }},
        }
    };
}

void TcpSysbotBase_Connection::write_data(const std::string& data){
    WriteSpinLock lg(m_send_lock);
    m_socket.blocking_send(data.data(), data.size());
}


std::string pretty_print(uint64_t x){
    auto ms = x / 1000;
    auto us = x - ms * 1000;
    std::string str_ms = std::to_string(ms);
    std::string str_us = std::to_string(us);
    return str_ms + "." + std::string(3 - str_us.size(), '0') + str_us;
}


void TcpSysbotBase_Connection::thread_loop(){
    std::unique_lock<std::mutex> lg(m_lock);
    WallClock send_time = current_time();
    while (true){
        ClientSocket::State state = m_socket.state();
        if (state == ClientSocket::State::DESTRUCTING || state == ClientSocket::State::NOT_RUNNING){
            break;
        }

        WallClock now = current_time();

        if (m_last_receive == WallClock::min()){
            send_time = now;
            write_data("getVersion\n");
        }else if (send_time < m_last_receive && now - m_last_receive < std::chrono::seconds(1)){
            std::chrono::microseconds latency = std::chrono::duration_cast<std::chrono::microseconds>(m_last_receive - send_time);
            std::string str = "Response Time: " + pretty_print(latency.count()) + " ms";
            if (latency < 10ms){
                set_status_line1(str, COLOR_BLUE);
            }else if (latency < 50ms){
                set_status_line1(str, COLOR_DARKGREEN);
            }else{
                set_status_line1(str, COLOR_ORANGE);
            }
            send_time = current_time();
            write_data("getVersion\n");
//            cout << std::chrono::duration_cast<std::chrono::microseconds>(current_time() - send_time) << endl;
        }else{
            std::chrono::milliseconds time_since = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_receive);
            std::string str = "Last Ack: " + pretty_print(time_since.count()) + " seconds ago";
            set_status_line1(str, COLOR_RED);
            send_time = current_time();
            write_data("getVersion\n");
//            cout << std::chrono::duration_cast<std::chrono::microseconds>(current_time() - send_time) << endl;
        }
        m_cv.wait_for(lg, std::chrono::seconds(1));
    }
}



void TcpSysbotBase_Connection::on_connect_finished(const std::string& error_message){
    try{
        if (!error_message.empty()){
            m_logger.log(m_connecting_message, COLOR_RED);
            set_status_line0(m_connecting_message, COLOR_RED);
            return;
        }

        m_logger.log(m_connecting_message + " (Success)", COLOR_BLUE);

        write_data("configure echoCommands 0\n");
        write_data("getVersion\n");
        write_data("configure mainLoopSleepTime 0\n");

        m_thread = std::thread(&TcpSysbotBase_Connection::thread_loop, this);

//        set_status_line0(m_version);

        declare_ready(controller_mode_status());
    }catch (...){}
}
void TcpSysbotBase_Connection::on_receive_data(const void* data, size_t bytes){
    WallClock now = current_time();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_last_receive = now;
    }
    try{
//        cout << "sys-botbase Response: " << std::string((const char*)data, bytes) << endl;

        //  Version #
        std::string str((const char*)data, bytes);
        if (str.find('.') != std::string::npos){
            while (!str.empty() && str.back() <= 32){
                str.pop_back();
            }
            set_status_line0("sys-botbase: Version " + str, COLOR_BLUE);
        }

    }catch (...){}
}









}
}
