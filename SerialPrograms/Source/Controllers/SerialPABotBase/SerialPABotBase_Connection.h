/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Connection_H
#define PokemonAutomation_Controllers_SerialPABotBase_Connection_H

#include <memory>
//#include <set>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Controllers/SerialPABotBase/Connection/BotBase.h"
#include "Controllers/SerialPABotBase/Connection/PABotBase.h"
#include "Controllers/SerialPABotBase/Connection/MessageLogger.h"
#include "Controllers/ControllerConnection.h"

namespace PokemonAutomation{
    class PABotBase;
    class BotBaseMessageType;
namespace SerialPABotBase{


class SerialPABotBase_Connection : public ControllerConnection{
public:
    SerialPABotBase_Connection(
        Logger& logger,
        const std::string& name,
        bool set_to_null_controller
    );
    ~SerialPABotBase_Connection();

    template <typename MessageType>
    void add_message_printer(){
        m_botbase->add_message_printer(BotBaseMessageType::instance<MessageType>());
    }


public:
    const std::string& device_name() const{
        return m_device_name;
    }
    uint32_t protocol_version() const{
        return m_protocol;
    }

    BotBaseController* botbase();

    ControllerType refresh_controller_type();


private:
    void add_message_printers();

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

    AsyncTask m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    mutable Mutex m_lock;
    Mutex m_sleep_lock;
    ConditionVariable m_cv;
};



}
}
#endif
