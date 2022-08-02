/*  Bot-Base Wrapper Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_BotBaseHandle_H
#define PokemonAutomation_CommonFramework_BotBaseHandle_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QObject>
#include <QSerialPortInfo>
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/MessageLogger.h"

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
        const QSerialPortInfo& port,
        PABotBaseLevel minimum_pabotbase
    );
    ~BotBaseHandle();

    void stop();
    void reset(const QSerialPortInfo& port);

    void set_allow_user_commands(bool allow);

public:
    //  Not thread-safe with stop() or reset(). The returned pointer
    //  will only be valid until the next call to stop()/reset().
    BotBase* botbase();

    //  Safe to call anywhere anytime. If called asynchronously with
    //  stop()/reset(), the result may be immediately out-of-date.
    State state() const;
    bool accepting_commands() const;

    std::string label() const;
    std::string status() const;

public:
    //  Async external requests. (typically from integration commands)
    //  Thread-safe with stop()/reset(). These may drop.
    const char* try_reset();
    const char* try_send_request(const BotBaseRequest& request);
    const char* try_stop_commands();
    const char* try_next_interrupt();

signals:
    void on_not_connected(std::string error);
    void on_connecting();
    void on_ready(std::string description);
    void on_stopped(std::string error);

    void uptime_status(std::string status);

private:
    const char* check_accepting_commands();

    void stop_unprotected();
    void reset_unprotected(const QSerialPortInfo& port);

    void verify_protocol();
    uint8_t verify_pabotbase();
    void thread_body();


private:
    SerialLogger& m_logger;

    const QSerialPortInfo m_port;
    PABotBaseLevel m_minimum_pabotbase;
    std::atomic<PABotBaseLevel> m_current_pabotbase;
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
