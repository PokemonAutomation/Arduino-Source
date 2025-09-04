/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Connection_H
#define PokemonAutomation_Controllers_SerialPABotBase_Connection_H

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
namespace SerialPABotBase{


class SerialPABotBase_Connection : public ControllerConnection{
public:
    SerialPABotBase_Connection(
        Logger& logger,
        const QSerialPortInfo* port,
        std::optional<ControllerType> change_controller,
        bool clear_settings
    );
    ~SerialPABotBase_Connection();


public:
//    uint8_t program_id() const{
//        return m_program_id;
//    }
    BotBaseController* botbase();

public:
    virtual ControllerModeStatus controller_mode_status() const override;


private:
    const std::map<uint32_t, std::vector<ControllerType>>&
    get_programs_for_protocol(uint32_t protocol);

    const std::vector<ControllerType>&
    get_controllers_for_program(
        const std::map<uint32_t, std::vector<ControllerType>>& available_programs,
        uint32_t program_id
    );

    void process_queue_size();
    ControllerType get_controller_type(
        const std::vector<ControllerType>& available_controllers
    );

    ControllerModeStatus process_device(
        std::optional<ControllerType> change_controller,
        bool clear_settings
    );

    void thread_body(
        std::optional<ControllerType> change_controller,
        bool clear_settings
    );


private:
    SerialLogger m_logger;
    std::string m_device_name;

    uint32_t m_protocol = 0;
    uint32_t m_version = 0;
    uint8_t m_program_id = 0;
    ControllerModeStatus m_mode_status;

    std::thread m_status_thread;
    std::unique_ptr<PABotBase> m_botbase;
    mutable std::mutex m_lock;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
};



}
}
#endif
