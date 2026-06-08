/*  Null Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_NullController_H
#define PokemonAutomation_Controllers_NullController_H

#include "ControllerDescriptor.h"
#include "ControllerConnection.h"

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

    virtual std::unique_ptr<ControllerConnection> open_connection(Logger& logger) const override;
    virtual std::unique_ptr<AbstractController> make_controller(
        Logger& logger,
        ControllerConnection& connection,
        ControllerType controller_type
    ) const override;

    virtual QWidget* make_selector_QtWidget(ControllerSelectorWidget& parent) const override;
};




class NullController final : public AbstractController{
public:
    static const char NAME[];

    NullController(Logger& logger)
        : m_logger(logger)
    {}

    virtual Logger& logger() override{
        return m_logger;
    }

    virtual const char* name() override{
        return NAME;
    }
    virtual ControllerClass controller_class() const override{
        return ControllerClass::None;
    }
    virtual ControllerPerformanceClass performance_class() const override{
        return ControllerPerformanceClass::Unknown;
    }
    virtual Milliseconds ticksize() const override{
        return Milliseconds(0);
    }
    virtual Milliseconds cooldown() const override{
        return Milliseconds(0);
    }
    virtual Milliseconds timing_variation() const override{
        return Milliseconds(0);
    }
    virtual bool atomic_multibutton() const override{
        return true;
    }
    virtual bool is_ready() const override{
        return true;
    }

    virtual void cancel_all_commands() override{}
    virtual void replace_on_next_command() override{}
    virtual void wait_for_all(Cancellable* cancellable) override{}
    virtual void issue_barrier(Cancellable* cancellable) override{}
    virtual void issue_nop(Cancellable* cancellable, Milliseconds duration) override{}


private:
    Logger& m_logger;
};









}
#endif
