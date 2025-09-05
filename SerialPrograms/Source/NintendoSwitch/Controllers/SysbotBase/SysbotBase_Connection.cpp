/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QEventLoop>
#include "Common/Cpp/Time.h"
//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
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
    , m_supports_command_queue(false)
    , m_last_ping_send(WallClock::min())
    , m_last_ping_receive(WallClock::min())
{
    m_controller_list.emplace_back(ControllerType::NintendoSwitch_WiredProController);
    m_current_controller = ControllerType::NintendoSwitch_WiredProController;

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
        write_data("detachController\r\n");
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


void TcpSysbotBase_Connection::write_data(const std::string& data){
    WriteSpinLock lg(m_send_lock, "TcpSysbotBase_Connection::write_data()");
//    cout << "Sending: " << data << endl;
    m_socket.send(data.data(), data.size());
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
    m_last_ping_send = current_time();
    while (true){
        ClientSocket::State state = m_socket.state();
        if (state == ClientSocket::State::DESTRUCTING || state == ClientSocket::State::NOT_RUNNING){
            break;
        }

        WallClock now = current_time();

        std::chrono::milliseconds time_since = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_ping_receive);
        if (time_since > std::chrono::seconds(5)){
            std::string str = "Last Ack: " + pretty_print(time_since.count()) + " seconds ago";
            set_status_line1(str, COLOR_RED);
        }

        m_last_ping_send = current_time();
        if (supports_command_queue() && NintendoSwitch::ConsoleSettings::instance().ENABLE_SBB3_PINGS){
            //  If we're more than 60 pings behind, just start clearing them.
            while (m_active_pings.size() > 60){
                m_active_pings.erase(m_active_pings.begin());
            }

            m_active_pings[m_ping_seqnum] = m_last_ping_send;
            write_data("ping " + std::to_string(m_ping_seqnum) + "\r\n");
            m_ping_seqnum++;
        }else{
            write_data("getVersion\r\n");
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

        write_data("configure echoCommands 0\r\n");
        write_data("getVersion\r\n");

//        m_thread = std::thread(&TcpSysbotBase_Connection::thread_loop, this);

//        set_status_line0(m_version);

//        declare_ready(controller_mode_status());
    }catch (...){}
}
void TcpSysbotBase_Connection::on_receive_data(const void* data, size_t bytes){
//    cout << "on_receive_data(): " << std::string((const char*)data, bytes - 2) << endl;

    WallClock now = current_time();

    try{
        const char* ptr = (const char*)data;
        for (size_t c = 0; c < bytes; c++){
            char ch = ptr[c];
            if (ch == '\r'){
                continue;
            }
            if (ch != '\n'){
                m_receive_buffer.emplace_back(ch);
                continue;
            }
            process_message(
                std::string(
                    m_receive_buffer.begin(),
                    m_receive_buffer.end()
                ),
                now
            );
            m_receive_buffer.clear();
        }

//        m_listeners.run_method_unique(&Listener::on_receive_data, data, bytes);

    }catch (...){}
}
void TcpSysbotBase_Connection::process_message(const std::string& message, WallClock timestamp){
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("Received: " + message, COLOR_DARKGREEN);
    }

    m_listeners.run_method_unique(&Listener::on_message, message);

    //  Version #
    std::string str = message;
    if (str.find('.') != std::string::npos){
        while (!str.empty() && str.back() <= 32){
            str.pop_back();
        }
        set_status_line0("sys-botbase: Version " + str, COLOR_BLUE);

        std::lock_guard<std::mutex> lg(m_lock);
        m_last_ping_receive = timestamp;

        if (m_last_ping_send != WallClock::min()){
            std::chrono::microseconds latency = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - m_last_ping_send);
            std::string text = "Response Time: " + pretty_print(latency.count()) + " ms";
            if (latency < 10ms){
                set_status_line1(text, COLOR_BLUE);
            }else if (latency < 50ms){
                set_status_line1(text, COLOR_DARKGREEN);
            }else{
                set_status_line1(text, COLOR_ORANGE);
            }
        }

        if (!m_thread.joinable()){
            set_mode(str);
        }
    }

    size_t pos = str.find("ping");
    if (pos != std::string::npos){
        const char* ptr = &str[pos + 4];
        while (true){
            char ch = ptr[0];
            if (ch < 32 || ch == ' ' || ch == '\t'){
                ptr++;
                continue;
            }
            break;
        }
        size_t ping_seqnum = atoll(ptr);

        std::lock_guard<std::mutex> lg(m_lock);
        m_last_ping_receive = timestamp;
        auto iter = m_active_pings.find(ping_seqnum);
        if (iter == m_active_pings.end()){
            m_logger.log("Received Unexpected Ping: " + std::to_string(ping_seqnum));
            return;
        }

        std::chrono::microseconds latency = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - iter->second);
        std::string text = "Response Time: " + pretty_print(latency.count()) + " ms";
        if (latency < 10ms){
            set_status_line1(text, COLOR_BLUE);
        }else if (latency < 50ms){
            set_status_line1(text, COLOR_DARKGREEN);
        }else{
            set_status_line1(text, COLOR_ORANGE);
        }

        m_active_pings.erase(iter);
    }

}
void TcpSysbotBase_Connection::set_mode(const std::string& sbb_version){
    if (sbb_version.rfind("2.", 0) == 0){
        m_logger.log("Detected sbb2. Using old (slow) command set.", COLOR_ORANGE);
        write_data("configure mainLoopSleepTime 0\r\n");
        m_supports_command_queue.store(false, std::memory_order_relaxed);
    }else if (sbb_version.rfind("3.", 0) == 0){
        m_logger.log("Detected sbb3. Using CC command queue.", COLOR_BLUE);
        if (NintendoSwitch::ConsoleSettings::instance().ENABLE_SBB3_LOGGING){
            write_data("configure enableLogs 1\r\n");
        }
        write_data("configure enablePA 1\r\n");
        m_supports_command_queue.store(true, std::memory_order_relaxed);
    }else{
        m_logger.log("Unrecognized sbb version: " + sbb_version, COLOR_RED);
        return;
    }

    m_thread = std::thread(&TcpSysbotBase_Connection::thread_loop, this);
    declare_ready();
}







}
}
