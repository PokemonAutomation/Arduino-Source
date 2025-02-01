/*  Serial Port (PABotBase) Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This file was mostly kept intact through the connections refactor. But it
 *  really needs to be redone as it's a mess.
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Handle_H
#define PokemonAutomation_Controllers_SerialPABotBase_Handle_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QObject>
#include <QSerialPortInfo>
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "Controllers/ControllerCapability.h"

namespace PokemonAutomation{

class MessageSniffer;
class PABotBase;


class BotBaseHandle : public QObject{
    Q_OBJECT
public:
    enum class State{
        NOT_CONNECTED,  //  Connection doesn't exist.
        CONNECTING,     //  Connection exists, but not ready.
        READY,          //  Connection exists and ready to use.
        ERRORED,
        STOPPED,        //  Connection exists, but has been stopped.
        SHUTDOWN,       //  Connection is in the process of being destroyed.
    };

public:
    //  Must call on main thread.
    BotBaseHandle(
        SerialLogger& logger,
        const QSerialPortInfo* port,
        const ControllerRequirements& requirements
    );
    ~BotBaseHandle();

    void stop();
    void reset(const QSerialPortInfo* port);

    void set_allow_user_commands(bool allow);

public:
    //  Not thread-safe with stop() or reset(). The returned pointer
    //  will only be valid until the next call to stop()/reset().
    BotBaseController* botbase();

    //  Safe to call anywhere anytime. If called asynchronously with
    //  stop()/reset(), the result may be immediately out-of-date.
    State state() const;
    bool accepting_commands() const;

    std::string label() const;
    const std::set<std::string>& capabilities() const{
        return m_capabilities;
    }


public:
    //  Async external requests. (typically from integration commands)
    //  Thread-safe with stop()/reset(). These may drop.
    const char* try_reset();
    const char* try_send_request(const BotBaseRequest& request);
    const char* try_stop_commands();
    const char* try_next_interrupt();

signals:
    void on_not_connected(std::string error);
    void on_connecting(const std::string& port_name);
    void on_ready(std::string description);
    void on_stopped(std::string error);

    void uptime_status(std::string status);

private:
    void process_device_protocol(uint32_t& version, uint8_t& program_id);

    const char* check_accepting_commands();

    void stop_unprotected();
    void reset_unprotected(const QSerialPortInfo* port);

    void thread_body();


private:
    SerialLogger& m_logger;

    const QSerialPortInfo* m_port;
    const ControllerRequirements& m_requirements;
    std::set<std::string> m_capabilities;

    std::atomic<State> m_state;
    std::atomic<bool> m_allow_user_commands;

    std::string m_label;

    std::thread m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    mutable std::mutex m_lock;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
};



}
#endif
