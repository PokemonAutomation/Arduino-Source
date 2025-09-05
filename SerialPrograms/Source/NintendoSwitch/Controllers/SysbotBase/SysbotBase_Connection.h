/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SysbotBase_Connection_H
#define PokemonAutomation_Controllers_SysbotBase_Connection_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QUrl>
#include <QHostAddress>
#include <QTcpSocket>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Sockets/ClientSocket.h"
#include "Controllers/ControllerConnection.h"

namespace PokemonAutomation{
namespace SysbotBase{


class TcpSysbotBase_Connection : public ControllerConnection, private ClientSocket::Listener{
public:
    struct Listener{
        virtual void on_message(const std::string& message) = 0;
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }

public:
    TcpSysbotBase_Connection(
        Logger& logger,
        const std::string& url
    );
    ~TcpSysbotBase_Connection();

    bool supports_command_queue() const{
        return m_supports_command_queue.load(std::memory_order_relaxed);
    }

    void write_data(const std::string& data);

private:
    void thread_loop();

    virtual void on_connect_finished(const std::string& error_message) override;
    virtual void on_receive_data(const void* data, size_t bytes) override;

    void process_message(const std::string& message, WallClock timestamp);
    void set_mode(const std::string& sbb_version);

private:
    Logger& m_logger;
    ClientSocket m_socket;

    std::atomic<bool> m_supports_command_queue;

    std::string m_connecting_message;
//    std::string m_version;
    WallClock m_last_ping_send;
    WallClock m_last_ping_receive;

    uint64_t m_ping_seqnum = 0;
    std::map<uint64_t, WallClock> m_active_pings;

    std::deque<char> m_receive_buffer;

    SpinLock m_send_lock;
    std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;

    ListenerSet<Listener> m_listeners;
};



}
}
#endif
