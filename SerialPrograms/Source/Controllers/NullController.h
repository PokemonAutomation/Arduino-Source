/*  Null Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_NullController_H
#define PokemonAutomation_Controllers_NullController_H

#include "ControllerDescriptor.h"

namespace PokemonAutomation{



class NullControllerDescriptor : public ControllerDescriptor{
public:
    static constexpr ControllerInterface INTERFACE_NAME = ControllerInterface::None;

public:
    NullControllerDescriptor()
        : ControllerDescriptor(ControllerInterface::None)
    {}
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
};



}
#endif
