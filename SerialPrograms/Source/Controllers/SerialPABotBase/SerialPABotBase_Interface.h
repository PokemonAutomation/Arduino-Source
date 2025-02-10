/*  Serial Port (PABotBase) Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Interface_H
#define PokemonAutomation_Controllers_SerialPABotBase_Interface_H

#include <QSerialPortInfo>
#include "Controllers/ControllerDescriptor.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class SerialPABotBase_Interface : public ControllerDescriptor
{
public:
    static const char* TYPENAME;

public:
    SerialPABotBase_Interface() = default;
    SerialPABotBase_Interface(const QSerialPortInfo& info)
        : m_port(info)
    {}

    const QSerialPortInfo& port() const{
        return m_port;
    }

    virtual bool operator==(const ControllerDescriptor& x) const override;
    virtual const char* type_name() const override;
    virtual std::string display_name() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<ControllerConnection> open_connection(
        Logger& logger
    ) const override;
    virtual std::unique_ptr<AbstractController> make_controller(
        Logger& logger,
        ControllerConnection& connection,
        ControllerType controller_type,
        const ControllerRequirements& requirements
    ) const override;

private:
    QSerialPortInfo m_port;
};



}
}
#endif
