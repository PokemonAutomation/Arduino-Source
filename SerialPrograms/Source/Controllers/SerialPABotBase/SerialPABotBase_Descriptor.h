/*  Serial Port (PABotBase) Descriptor
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Descriptor_H
#define PokemonAutomation_Controllers_SerialPABotBase_Descriptor_H

#include <QSerialPortInfo>
#include "Controllers/Controllers.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


class SerialDescriptor : public ControllerDescriptor{
public:
    static const char TYPENAME[];

public:
    virtual std::unique_ptr<ControllerDescriptor> clone() const override;
    virtual bool operator==(const ControllerDescriptor& x) const override;

public:
    SerialDescriptor() = default;
    SerialDescriptor(const QSerialPortInfo& info)
        : m_port(info)
    {}

    const QSerialPortInfo& port() const{
        return m_port;
    }


    virtual const char* type_name() const override;
    virtual std::string display_name() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<ControllerConnection> open(
        Logger& logger,
        const ControllerRequirements& requirements
    ) const override;

private:
    QSerialPortInfo m_port;
};


std::vector<std::unique_ptr<const ControllerDescriptor>> get_all_devices();





}
}
#endif
