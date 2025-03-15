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
        std::optional<ControllerType> change_controller
    );
    ~SerialPABotBase_Connection();

    void update_with_capabilities(const ControllerFeatures& capabilities);


public:
    BotBaseController* botbase();

public:

    virtual ControllerModeStatus controller_mode_status() const override;


private:
    ControllerModeStatus read_device_specs(
        std::optional<ControllerType> change_controller
    );

    void thread_body(
        std::optional<ControllerType> change_controller
    );


private:
    SerialLogger m_logger;
    std::string m_device_name;

    uint32_t m_protocol = 0;
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
