/*  Serial Port (PABotBase) Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Descriptor_H
#define PokemonAutomation_Controllers_SerialPABotBase_Descriptor_H

#include <QSerialPortInfo>
#include "Controllers/ControllerDescriptor.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class SerialPABotBase_Descriptor : public ControllerDescriptor{
public:
    static constexpr ControllerInterface INTERFACE_NAME = ControllerInterface::SerialPABotBase;


public:
    SerialPABotBase_Descriptor()
        : ControllerDescriptor(INTERFACE_NAME)
    {}
    SerialPABotBase_Descriptor(const QSerialPortInfo& info)
        : ControllerDescriptor(INTERFACE_NAME)
        , m_port(info)
    {}

    const QSerialPortInfo& port() const{
        return m_port;
    }

    virtual bool operator==(const ControllerDescriptor& x) const override;
    virtual std::string display_name() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<ControllerConnection> open_connection(
        Logger& logger,
        bool set_to_null_controller
    ) const override;
    virtual std::unique_ptr<AbstractController> make_controller(
        Logger& logger,
        ControllerConnection& connection,
        ControllerType controller_type,
        ControllerResetMode reset_mode
    ) const override;

    virtual QWidget* make_selector_QtWidget(ControllerSelectorWidget& parent) const override;

private:
    QSerialPortInfo m_port;
};



}
}
#endif
