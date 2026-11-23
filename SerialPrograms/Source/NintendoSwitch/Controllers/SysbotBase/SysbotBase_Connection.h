/*  sys-botbase Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SysbotBase_Connection_H
#define PokemonAutomation_Controllers_SysbotBase_Connection_H

#include <deque>
#include <QUrl>
#include <QHostAddress>
#include <QTcpSocket>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Sockets/ClientSocket.h"
#include "Controllers/ControllerConnection.h"

namespace PokemonAutomation{

class UnitTestDatabase;

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

    //  Sends "data" on the connection, waiting at most "timeout" for
    //  the peer to read. If the deadline elapses, the connection is
    //  left alone and the number of bytes actually written is
    //  returned. The default (no deadline) preserves the historical
    //  behavior of the command and setup paths; the shutdown and
    //  heartbeat paths pass a short deadline so that a peer that stops
    //  reading cannot stall destruction or hold the send lock.
    void write_data(const std::string& data, std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

    //
    //  Parses the sequence number that follows a "ping" token in "str",
    //  setting "seqnum" and returning true only if a complete, in-range
    //  unsigned integer is present. The scan is bounded to "str", so a
    //  message that ends right after "ping" (or has no valid number after
    //  it) returns false instead of reading out of bounds.
    //
    static bool parse_ping_response(const std::string& str, uint64_t& seqnum);

private:
    void thread_loop();

    virtual void on_connect_finished(const std::string& error_message) override;
    virtual void on_receive_data(const void* data, size_t bytes) override;
    virtual void on_disconnect(const std::string& error_message) override;

    void process_message(const std::string& message, WallClock timestamp);
    void on_receive_overflow();
    void set_mode(const std::string& sbb_version);

private:
    Logger& m_logger;
    ClientSocket m_socket;

    std::string m_connecting_message;
//    std::string m_version;
    WallClock m_last_ping_send;
    WallClock m_last_ping_receive;

    uint64_t m_ping_seqnum = 0;
    std::map<uint64_t, WallClock> m_active_pings;

    std::deque<char> m_receive_buffer;

    SpinLock m_send_lock;
    Mutex m_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;

    ListenerSet<Listener> m_listeners;
};

//
//  Registers unit tests for parse_ping_response().
//
void add_tests_SysbotBasePing(UnitTestDatabase& database);

//
//  Registers unit tests for the receive-line length bound.
//
void add_tests_SysbotBaseReceiveLine(UnitTestDatabase& database);

//
//  Registers the unit test for the bounded shutdown send.
//
void add_tests_SysbotBaseShutdown(UnitTestDatabase& database);



}
}
#endif
