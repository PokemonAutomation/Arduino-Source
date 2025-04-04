/*  SerialPABotBase: Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_PokkenController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_PokkenController_H

#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_PokkenController final :
    public ProController,
    public SerialPABotBase_Controller
{
public:
    using ContextType = ProControllerContext;


public:
    SerialPABotBase_PokkenController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection
    );
    ~SerialPABotBase_PokkenController();
    void stop();

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
        return ControllerType::NintendoSwitch_WiredProController;
    }
    virtual const ControllerFeatures& controller_features() const override{
        return m_supported_features;
    }
    virtual ControllerPerformanceClass performance_class() const override{
        return ControllerPerformanceClass::SerialPABotBase_Wired_125Hz;
    }

    virtual Milliseconds ticksize() const override{
        return Milliseconds(8);
    }
    virtual Milliseconds cooldown() const override{
        return Milliseconds(8);
    }
    virtual Milliseconds timing_variation() const override{
        return Milliseconds::zero();
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
        button &= VALID_PRO_CONTROLLER_BUTTONS;
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
        button &= VALID_PRO_CONTROLLER_BUTTONS;
        ControllerWithScheduler::issue_mash_button(cancellable, button, duration);
    }
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, Milliseconds duration
    ) override{
        button0 &= VALID_PRO_CONTROLLER_BUTTONS;
        button1 &= VALID_PRO_CONTROLLER_BUTTONS;
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
    template <typename Type>
    PA_FORCE_INLINE Type milliseconds_to_ticks_8ms(Type milliseconds){
        return milliseconds / 8 + (milliseconds % 8 + 7) / 8;
    }
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;

    void status_thread();


private:
    CancellableHolder<CancellableScope> m_scope;
    std::atomic<bool> m_stopping;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_status_thread;
};




}
}
#endif
