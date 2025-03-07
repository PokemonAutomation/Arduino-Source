/*  SerialPABotBase: Wireless Joycon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessJoycon_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessJoycon_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"
#include "NintendoSwitch_SerialPABotBase_WirelessController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_WirelessJoycon final :
    public JoyconController,
    public SerialPABotBase_WirelessController
{
public:
    SerialPABotBase_WirelessJoycon(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerType controller_type
    );
    ~SerialPABotBase_WirelessJoycon();

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual bool is_ready() const override{
        return SerialPABotBase_Controller::is_ready();
    }


public:
    virtual ControllerType controller_type() const override{
        return m_controller_type;
    }
    virtual const ControllerFeatures& controller_features() const override{
        return m_supported_features;
    }

    virtual Milliseconds ticksize() const override{
        return SerialPABotBase_WirelessController::ticksize();
    }
    virtual Milliseconds timing_variation() const override{
        return SerialPABotBase_WirelessController::timing_variation();
    }


public:
    virtual void cancel_all_commands() override{
        SerialPABotBase_Controller::cancel_all_commands();
    }
    virtual void replace_on_next_command() override{
        SerialPABotBase_Controller::replace_on_next_command();
    }

    virtual void wait_for_all(const Cancellable* cancellable) override{
        SerialPABotBase_Controller::wait_for_all(cancellable);
    }


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(const Cancellable* cancellable) override{
        ControllerWithScheduler::issue_barrier(cancellable);
    }
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) override{
        ControllerWithScheduler::issue_nop(cancellable, duration);
    }
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Button button,
        uint8_t joystick_x, uint8_t joystick_y,
        Milliseconds hold
    ) override;


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button, Milliseconds duration
    ) override;


private:
    void push_state_left_joycon(const Cancellable* cancellable, WallDuration duration);
    void push_state_right_joycon(const Cancellable* cancellable, WallDuration duration);
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;

    ControllerType m_controller_type;
    Button m_valid_buttons;
};



}
}
#endif
