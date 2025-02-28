/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SysbotBase_Connection_H
#define PokemonAutomation_Controllers_SysbotBase_Connection_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <QUrl>
#include <QHostAddress>
#include <QTcpSocket>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Sockets/ClientSocket.h"
#include "Controllers/ControllerConnection.h"

#define PA_USE_NATIVE_SOCKET

namespace PokemonAutomation{
namespace SysbotBase{


#ifdef PA_USE_NATIVE_SOCKET
class TcpSysbotBase_Connection : public ControllerConnection, private ClientSocket::Listener{
public:
    TcpSysbotBase_Connection(
        Logger& logger,
        const std::string& url
    );
    ~TcpSysbotBase_Connection();

    virtual std::map<ControllerType, std::set<ControllerFeature>> supported_controllers() const override;

    void write_data(const std::string& data);

private:
    void thread_loop();

    virtual void on_connect_finished(const std::string& error_message) override;
    virtual void on_receive_data(const void* data, size_t bytes) override;

private:
    Logger& m_logger;
    ClientSocket m_socket;

    std::string m_connecting_message;
    std::string m_version;
    WallClock m_last_receive;

    SpinLock m_send_lock;
    std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};


#else
class TcpSysbotBase_Connection : public ControllerConnection{
public:
    TcpSysbotBase_Connection(
        Logger& logger,
        const std::string& url
    );
    ~TcpSysbotBase_Connection();

    virtual std::map<ControllerType, std::set<ControllerFeature>> supported_controllers() const override;

    void write_data(const std::string& data);

private:
    void thread_body_wrapper();
    void thread_body_internal();

private:
    Logger& m_logger;
    QHostAddress m_address;
    int m_port;

    QTcpSocket* m_socket = nullptr;

    std::atomic<bool> m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};

#endif


}
}
#endif
