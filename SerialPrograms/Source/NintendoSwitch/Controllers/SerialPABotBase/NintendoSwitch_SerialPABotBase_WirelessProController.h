/*  SerialPABotBase: Wireless Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessProController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessProController_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"
#include "NintendoSwitch_SerialPABotBase_WirelessController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_WirelessProController final :
    public ProController,
    public SerialPABotBase_WirelessController
{
    static constexpr uint16_t JOYSTICK_MIN_THRESHOLD = 1897;
    static constexpr uint16_t JOYSTICK_MAX_THRESHOLD = 320;

public:
    SerialPABotBase_WirelessProController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection
    );
    ~SerialPABotBase_WirelessProController();

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual RecursiveThrottler& logging_throttler() override{
        return m_logging_throttler;
    }
    virtual bool is_ready() const override{
        return SerialPABotBase_Controller::is_ready();
    }


public:
    virtual ControllerType controller_type() const override{
        return ControllerType::NintendoSwitch_WirelessProController;
    }
    virtual const ControllerFeatures& controller_features() const override{
        return m_supported_features;
    }
    virtual ControllerPerformanceClass performance_class() const override{
        return ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32;
    }

    virtual Milliseconds ticksize() const override{
        return SerialPABotBase_WirelessController::ticksize();
    }
    virtual Milliseconds cooldown() const override{
        return SerialPABotBase_WirelessController::cooldown();
    }
    virtual Milliseconds timing_variation() const override{
        return SerialPABotBase_WirelessController::timing_variation();
    }
    virtual bool atomic_multibutton() const override{
        return true;
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

    virtual void send_botbase_request(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) override{
        SerialPABotBase_Controller::send_botbase_request(cancellable, request);
    }
    virtual BotBaseMessage send_botbase_request_and_wait(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) override{
        return SerialPABotBase_Controller::send_botbase_request_and_wait(cancellable, request);
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
    ) override{
        ControllerWithScheduler::issue_buttons(cancellable, button, delay, hold, cooldown);
    }
    virtual void issue_dpad(
        const Cancellable* cancellable,
        DpadPosition position,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_dpad(cancellable, position, delay, hold, cooldown);
    }
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_left_joystick(cancellable, x, y, delay, hold, cooldown);
    }
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_right_joystick(cancellable, x, y, delay, hold, cooldown);
    }
    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        Milliseconds hold
    ) override{
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            button,
            position,
            left_x, left_y,
            right_x, right_y,
            hold
        );
    }


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button, Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, button, duration);
    }
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, button0, button1, duration);
    }
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_AZs(cancellable, duration);
    }
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_system_scroll(cancellable, direction, delay, hold, cooldown);
    }


private:
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;
};



}
}
#endif
