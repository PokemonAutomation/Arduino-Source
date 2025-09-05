/*  sys-botbase Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SysbotBase_Descriptor_H
#define PokemonAutomation_Controllers_SysbotBase_Descriptor_H

#include "Controllers/ControllerDescriptor.h"

namespace PokemonAutomation{
namespace SysbotBase{



class TcpSysbotBase_Descriptor : public ControllerDescriptor{
public:
    static constexpr ControllerInterface INTERFACE_NAME = ControllerInterface::TcpSysbotBase;


public:
    TcpSysbotBase_Descriptor()
        : ControllerDescriptor(INTERFACE_NAME)
    {}
    TcpSysbotBase_Descriptor(std::string url)
        : ControllerDescriptor(INTERFACE_NAME)
        , m_url(std::move(url))
    {}

    const std::string& url() const{
        return m_url;
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
    std::string m_url;
};







}
}
#endif
