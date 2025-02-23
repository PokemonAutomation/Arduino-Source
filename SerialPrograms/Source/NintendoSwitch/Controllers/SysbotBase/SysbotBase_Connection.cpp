/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Time.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "SysbotBase_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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
    if (m_socket == nullptr){
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
#if 1
    m_socket->connect(
        m_socket, &QTcpSocket::disconnected,
        m_socket, [this]{
            {
                std::unique_lock<std::mutex> lg(m_lock);
                m_ready.store(false, std::memory_order_release);
                m_stopping.store(true, std::memory_order_release);
                m_cv.notify_all();
            }
            set_status(html_color_text("Disconnected by host.", COLOR_RED));
        }
    );
#endif

    m_socket->connectToHost(m_address, m_port);

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

    m_socket->write("configure mainLoopSleepTime 0\n");
    m_socket->flush();

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

    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{
        return m_stopping.load(std::memory_order_relaxed);
    });
}





}
}
