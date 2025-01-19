/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Connection_H
#define PokemonAutomation_Controllers_SerialPABotBase_Connection_H

#include "Controllers/ControllerConnection.h"
#include "SerialPABotBase_Handle.h"
#include "SerialPABotBase_Descriptor.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


class SerialConnection : public QObject, public ControllerConnection{
public:
    SerialConnection(
        Logger& logger,
        const SerialDescriptor& descriptor,
        const ControllerRequirements& requirements
    );

    BotBaseHandle& handle(){ return m_handle; }

public:
    virtual std::string stop_pending_commands() override;
    virtual std::string set_next_command_replace() override;
    virtual std::string send_request(const BotBaseRequest& request) override;

private:
    void update_status_string();

private:
    SerialLogger m_logger;
    BotBaseHandle m_handle;

    std::string m_status;
    std::string m_uptime;
};



}
}
#endif
