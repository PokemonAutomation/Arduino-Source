/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Connection_H
#define PokemonAutomation_Controllers_SerialPABotBase_Connection_H

#include <memory>
#include <set>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "Controllers/ControllerConnection.h"

class QSerialPortInfo;

namespace PokemonAutomation{
    class PABotBase;
namespace SerialPABotBase{


class SerialPABotBase_Connection : public ControllerConnection{
public:
    SerialPABotBase_Connection(
        Logger& logger,
        const QSerialPortInfo* port,
        bool set_to_null_controller
    );
    ~SerialPABotBase_Connection();


public:
    const std::string& device_name() const{
        return m_device_name;
    }
    BotBaseController* botbase();

    ControllerType refresh_controller_type();


private:
    void process_queue_size();
    void throw_incompatible_protocol();
    ControllerType process_device(bool set_to_null_controller);

    void thread_body(bool set_to_null_controller);


private:
    SerialLogger m_logger;
    std::string m_device_name;

    uint32_t m_protocol = 0;
    uint32_t m_version = 0;
    uint8_t m_program_id = 0;
    std::string m_program_name;

    std::thread m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    mutable std::mutex m_lock;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
};



}
}
#endif
