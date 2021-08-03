/*  Bot-Base Wrapper Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_BotBaseHandle_H
#define PokemonAutomation_BotBaseHandle_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QObject>
#include <QSerialPortInfo>
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "ClientSource/Connection/BotBase.h"

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
        STOPPED,        //  Connection exists, but has beeb stopped.
        SHUTDOWN,       //  Connection is in the process of being destroyed.
    };

public:
    //  Must call on main thread.
    BotBaseHandle(
        const QSerialPortInfo& port,
        PABotBaseLevel minimum_pabotbase,
        MessageSniffer& logger
    );
    ~BotBaseHandle();

    void stop();
    void reset(const QSerialPortInfo& port);

public:
    //  Not thread-safe with stop() or reset(). The returned pointer
    //  will only be valid until the next call to stop()/reset().
    BotBase* botbase();

    //  Safe to call anywhere anytime. If called asynchronously with
    //  stop()/reset(), the result may be immediately out-of-date.
    State state() const;
    bool accepting_commands() const;

    //  Must call this on the same thread that controls the lifetime
    //  of this class. Not thread-safe with stop()/reset().
    template <uint8_t SendType, typename Parameters>
    bool try_send_request(Parameters& params);
    bool try_send_request(uint8_t msg_type, void* params, size_t bytes);

    //  This one can be called from anywhere.
    template <uint8_t SendType, typename Parameters>
    void async_try_send_request(Parameters& params);
    void async_try_send_request(uint8_t msg_type, void* params, size_t bytes);

signals:
    void on_not_connected(QString error);
    void on_connecting();
    void on_ready(QString description);
    void on_stopped(QString error);

    void uptime_status(QString status);

    void async_try_send_request(uint8_t msg_type, std::string body);

private:
    void stop_unprotected();

    void verify_protocol();
    uint8_t verify_pabotbase();
    void thread_body();


private:
    PABotBaseLevel m_minimum_pabotbase;
    std::atomic<PABotBaseLevel> m_current_pabotbase;
    std::atomic<State> m_state;

    MessageSniffer& m_logger;

    std::thread m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    std::mutex m_lock;
    std::mutex m_cv_lock;
    std::condition_variable m_cv;
};



template <uint8_t SendType, typename Parameters>
bool BotBaseHandle::try_send_request(Parameters& params){
    return try_send_request(SendType, &params, sizeof(Parameters));
}
template <uint8_t SendType, typename Parameters>
void BotBaseHandle::async_try_send_request(Parameters& params){
    async_try_send_request(SendType, &params, sizeof(Parameters));
}


}
#endif
