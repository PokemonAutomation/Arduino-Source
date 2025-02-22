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
#include "Controllers/ControllerConnection.h"

namespace PokemonAutomation{
namespace SysbotBase{


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




}
}
#endif
