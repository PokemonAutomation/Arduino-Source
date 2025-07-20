/*  SerialPABotBase: Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_PokkenController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_PokkenController_H

#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
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
        return ControllerPerformanceClass::SerialPABotBase_Wired;
    }

    virtual Milliseconds ticksize() const override{
        return Milliseconds(0);
    }
    virtual Milliseconds cooldown() const override{
        return Milliseconds(8);
    }
    virtual Milliseconds timing_variation() const override{
        return ConsoleSettings::instance().TIMING_OPTIONS.WIRED_MICROCONTROLLER;
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
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    ) override{
        button &= VALID_PRO_CONTROLLER_BUTTONS;
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
    }
    virtual void issue_dpad(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    ) override{
        ControllerWithScheduler::issue_dpad(cancellable, delay, hold, cooldown, position);
    }
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) override{
        ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, x, y);
    }
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) override{
        ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, x, y);
    }

    virtual void issue_gyro_accel_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_x(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_accel_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_y(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_accel_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_z(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_x(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_y(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_z(cancellable, delay, hold, cooldown, value);
    }

    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Milliseconds hold,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y
    ) override{
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            hold,
            button,
            position,
            left_x, left_y,
            right_x, right_y
        );
    }


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button
    ) override{
        button &= VALID_PRO_CONTROLLER_BUTTONS;
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button);
    }
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button0, Button button1
    ) override{
        button0 &= VALID_PRO_CONTROLLER_BUTTONS;
        button1 &= VALID_PRO_CONTROLLER_BUTTONS;
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button0, button1);
    }
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_AZs(cancellable, duration);
    }
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    ) override{
        ControllerWithScheduler::issue_system_scroll(cancellable, delay, hold, cooldown, direction);
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
