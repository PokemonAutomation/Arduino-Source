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




#ifdef PA_USE_NATIVE_SOCKET

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
        set_status(html_color_text("Invalid IP address + port.", COLOR_RED));
        return;
    }

    set_status(html_color_text("Connecting...", COLOR_DARKGREEN));

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


std::map<ControllerType, std::set<ControllerFeature>> TcpSysbotBase_Connection::supported_controllers() const{
    return {
        {ControllerType::NintendoSwitch_WiredProController, {
            ControllerFeature::NintendoSwitch_ProController
        }},
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
                set_status(html_color_text(m_version + str, COLOR_BLUE));
            }else if (latency < 50ms){
                set_status(html_color_text(m_version + str, COLOR_ORANGE));
            }else{
                set_status(html_color_text(m_version + str, COLOR_RED));
            }
            send_time = current_time();
            write_data("getVersion\n");
//            cout << std::chrono::duration_cast<std::chrono::microseconds>(current_time() - send_time) << endl;
        }else{
            std::chrono::milliseconds time_since = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_receive);
            std::string str = "Last Ack: " + pretty_print(time_since.count()) + " seconds ago";
            set_status(html_color_text(m_version + str, COLOR_RED));
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
            set_status(html_color_text(m_connecting_message, COLOR_RED));
            return;
        }

        m_logger.log(m_connecting_message + " (Success)", COLOR_BLUE);

        write_data("configure echoCommands 0\n");
        write_data("getVersion\n");
        write_data("configure mainLoopSleepTime 0\n");

        m_thread = std::thread(&TcpSysbotBase_Connection::thread_loop, this);

        set_status(m_version);

        declare_ready(supported_controllers());
    }catch (...){}
}
void TcpSysbotBase_Connection::on_receive_data(const void* data, size_t bytes){
    WallClock now = current_time();
    std::lock_guard<std::mutex> lg(m_lock);
    try{
        m_last_receive = now;
//        cout << "sys-botbase Response: " << std::string((const char*)data, bytes) << endl;

        if (m_version.empty()){
            std::string str((const char*)data, bytes);
            if (str.find('.') != std::string::npos){
                while (!str.empty() && str.back() <= 32){
                    str.pop_back();
                }
                m_version = html_color_text("sys-botbase: Version " + str, COLOR_BLUE) + "<br>";
                m_cv.notify_all();
            }
        }

    }catch (...){}
}






#else
TcpSysbotBase_Connection::TcpSysbotBase_Connection(
    Logger& logger,
    const std::string& url
)
    : m_logger(logger)
{
    if (!parse_ip_and_port(url, m_address, m_port)){
        set_status(html_color_text("Invalid IP address + port.", COLOR_RED));
        return;
    }

    set_status(html_color_text("Connecting...", COLOR_DARKGREEN));

    m_thread = std::thread(&TcpSysbotBase_Connection::thread_body_wrapper, this);
}
TcpSysbotBase_Connection::~TcpSysbotBase_Connection(){
    m_ready.store(false, std::memory_order_release);
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_lock);
//        if (m_socket){
//            m_socket->abort();
//        }
        m_cv.notify_all();
    }
    if (m_thread.joinable()){
        m_thread.join();
    }
}


std::map<ControllerType, std::set<ControllerFeature>> TcpSysbotBase_Connection::supported_controllers() const{
    return {
        {ControllerType::NintendoSwitch_WiredProController, {
            ControllerFeature::NintendoSwitch_ProController
        }},
    };
}



void TcpSysbotBase_Connection::write_data(const std::string& data){
    std::lock_guard<std::mutex> lg(m_lock);
    if (!m_socket){
        return;
    }
//    cout << "Sending: " << data << endl;
    qint64 written = m_socket->write(data.data(), data.size());
    m_socket->flush();
//    cout << "written = " << written << endl;
    if (written < 0 || written < (qint64)data.size()){
        m_ready.store(false, std::memory_order_release);
        m_stopping.store(true, std::memory_order_release);
        m_cv.notify_all();
    }
}



void TcpSysbotBase_Connection::thread_body_wrapper(){
    QTcpSocket socket;
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_socket = &socket;
    }
#if 0
    thread_body_internal();
#else
    try{
        thread_body_internal();
    }catch (Exception& e){
        try{
            global_logger_tagged().log("An exception was thrown from the network thread: " + e.to_str(), COLOR_RED);
            set_status(html_color_text("Error: Unhandled Exception", COLOR_RED));
        }catch (...){}
    }catch (std::exception& e){
        try{
            global_logger_tagged().log("An exception was thrown from the network thread: " + std::string(e.what()), COLOR_RED);
            set_status(html_color_text("Error: Unhandled Exception", COLOR_RED));
        }catch (...){}
    }catch (...){
        try{
            global_logger_tagged().log("An exception was thrown from the network thread.", COLOR_RED);
            set_status(html_color_text("Error: Unhandled Exception", COLOR_RED));
        }catch (...){}
    }
#endif

    try{
        m_socket->write("detachController\n");
        m_socket->flush();
        socket.close();
    }catch (...){}

    std::lock_guard<std::mutex> lg(m_lock);
    m_socket = nullptr;
}
void TcpSysbotBase_Connection::thread_body_internal(){
    std::string message =
        "Connecting To: " + m_address.toString().toStdString() +
        " - Port: " + std::to_string(m_port);

    m_logger.log(message);


    m_socket->connectToHost(m_address, m_port);
#if 1
#if 0
    m_socket->connect(
        m_socket, &QTcpSocket::disconnected,
        m_socket, [this]{
            cout << "QTcpSocket::disconnected()" << endl;
#if 0
            {
                std::unique_lock<std::mutex> lg(m_lock);
                m_ready.store(false, std::memory_order_release);
                m_stopping.store(true, std::memory_order_release);
                m_cv.notify_all();
            }
            set_status(html_color_text("Disconnected by host.", COLOR_RED));
#endif
        }
    );
#endif
    m_socket->connect(
        m_socket, &QIODevice::readyRead,
        m_socket, [this]{
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.notify_all();
        }
    );
#endif


    WallClock start = current_time();
    WallClock deadline = start + std::chrono::seconds(30);
    while (true){
        if (current_time() > deadline){
            m_logger.log("Connection timed out.", COLOR_RED);
            set_status(html_color_text("Connection timed out.", COLOR_RED));
            return;
        }
        if (m_stopping.load(std::memory_order_acquire)){
            m_logger.log("Connection aborted.", COLOR_RED);
            return;
        }
        if (m_socket->waitForConnected(50)){
            break;
        }
    }

    m_logger.log(message + " (Success)", COLOR_BLUE);
    set_status(html_color_text("Connected!", COLOR_BLUE));

    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_socket->write("configure echoCommands 1\n");
        m_socket->write("configure mainLoopSleepTime 0\n");
        m_socket->flush();
    }

    declare_ready(supported_controllers());

    //
    //  This is stupid. Since we cannot block the constructor of this class to
    //  wait for the connection, we spawn this thread to do the connection.
    //  This thread is no longer needed once we've connected and have reported
    //  the status.
    //
    //  However, QTcpSocket doesn't like being constructed on one thread
    //  and connecting out from another. So we construct it in this thread.
    //  It also means we cannot destruct it until we're done using it. Thus we
    //  must keep this thread alive until the entire class dies.
    //
    //  If we ever need to receive packets from this connection, this would be
    //  the thread to do it on.
    //

#if 0
    QEventLoop eventLoop;
//    char buffer[4096];

    WallClock next_ping = current_time();

    std::unique_lock<std::mutex> lg(m_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        eventLoop.processEvents();

//        auto read = m_socket->readAll();
//        cout << "sys-botbase Sent: " << read.size() << endl;
        qint64 bytes = m_socket->read(buffer, 4096);
        cout << "bytes = " << bytes << endl;
        if (bytes > 0){
            cout << "sys-botbase Sent: " << std::string(buffer, bytes) << endl;
        }

        WallClock now = current_time();
        if (now >= next_ping){
//            cout << "requesting version" << endl;
            m_socket->write("getVersion\n");
            m_socket->flush();
            next_ping = now + 1000ms;
        }

        m_cv.wait_until(lg, next_ping);
    }
#endif

#if 1
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{
        return m_stopping.load(std::memory_order_relaxed);
    });
#endif
}


#endif




}
}
