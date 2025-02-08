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
#include <QSerialPortInfo>
#include "Common/Cpp/ListenerSet.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "Controllers/ControllerCapability.h"

namespace PokemonAutomation{

class MessageSniffer;
class PABotBase;


class BotBaseHandle{
public:
    struct Listener{
        virtual void pre_not_ready(){}
        virtual void post_ready(const std::set<std::string>& capabilities){}
        virtual void post_status_text_changed(const std::string& text){}
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


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

//    void set_allow_user_commands(bool allow);

public:
    //  Not thread-safe with stop() or reset(). The returned pointer
    //  will only be valid until the next call to stop()/reset().
    BotBaseController* botbase();

    //  Safe to call anywhere anytime. If called asynchronously with
    //  stop()/reset(), the result may be immediately out-of-date.
    State state() const;

    std::string status_text() const;


public:
    //  Async external requests. (typically from integration commands)
    //  Thread-safe with stop()/reset(). These may drop.
    const char* try_reset();
    const char* try_send_request(const BotBaseRequest& request);
    const char* try_stop_commands();
    const char* try_next_interrupt();


private:
    void process_device_protocol(uint32_t& protocol, uint8_t& program_id);

    void stop_unprotected();
    void reset_unprotected(const QSerialPortInfo* port);

    void set_label_text(const std::string& text, Color color = Color());
    void set_uptime_text(const std::string& text, Color color);

    void thread_body();


private:
    SerialLogger& m_logger;

    const QSerialPortInfo* m_port;
    const ControllerRequirements& m_requirements;
    std::set<std::string> m_capabilities;

    std::atomic<State> m_state;

    std::string m_label;
    std::string m_uptime;
    std::string m_status_text;

    std::thread m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    mutable std::mutex m_lock;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;

    ListenerSet<Listener> m_listeners;
};



}
#endif
