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
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "Controllers/ControllerConnection.h"

class QSerialPortInfo;

namespace PokemonAutomation{

class PABotBase;


class BotBaseHandle : public ControllerConnection{
public:
    BotBaseHandle(SerialLogger& logger, const QSerialPortInfo* port);
    ~BotBaseHandle();


public:
    BotBaseController* botbase();

    //  Note that this is asynchronous. The returned value may be out-of-date
    //  immediately.
    bool is_ready() const;

public:
    std::set<std::string> capabilities() const{
        std::lock_guard<std::mutex> lg(m_lock);
        return m_capabilities;
    }


private:
    std::set<std::string> read_device_specs(uint32_t& protocol, uint8_t& program_id);

    void set_label_text(const std::string& text, Color color = Color());
    void set_uptime_text(const std::string& text, Color color);

    void thread_body();


private:
    SerialLogger& m_logger;

    const QSerialPortInfo* m_port;
    std::set<std::string> m_capabilities;

    std::atomic<bool> m_ready;

    std::string m_label;
    std::string m_uptime;

    std::thread m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    mutable std::mutex m_lock;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
};



}
#endif
