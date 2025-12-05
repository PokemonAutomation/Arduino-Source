/*  SerialPABotBase: Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_Joycon_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_Joycon_H

#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"
#include "NintendoSwitch_SerialPABotBase_OemController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename JoyconType>
class SerialPABotBase_Joycon :
    public JoyconType,
    public SerialPABotBase_OemController
{
    static constexpr uint16_t JOYSTICK_MIN_THRESHOLD = 1874;
    static constexpr uint16_t JOYSTICK_MAX_THRESHOLD = 260;

protected:
    SerialPABotBase_Joycon(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerType controller_type,
        ControllerResetMode reset_mode
    );


public:
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
//    virtual ControllerType controller_type() const override{
//        return m_controller_type;
//    }

    virtual ControllerPerformanceClass performance_class() const override{
        return m_performance_class;
    }
    virtual Milliseconds ticksize() const override{
        return m_ticksize;
    }
    virtual Milliseconds cooldown() const override{
        return m_cooldown;
    }
    virtual Milliseconds timing_variation() const override{
        return m_timing_variation;
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
    ) override;
    virtual void issue_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    ) override;

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
        bool enable_logging,
        Milliseconds duration,
        Button button,
        uint8_t joystick_x, uint8_t joystick_y
    ) override;


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button, Milliseconds duration
    ) override;


protected:
    void execute_state_left_joycon(
        const Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    );
    void execute_state_right_joycon(
        const Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    );
    virtual void execute_state(
        const Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override;

    ControllerType m_controller_type;
    Button m_valid_buttons;
};



class SerialPABotBase_LeftJoycon final : public SerialPABotBase_Joycon<LeftJoycon>{
public:
    SerialPABotBase_LeftJoycon(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerResetMode reset_mode
    );
    ~SerialPABotBase_LeftJoycon();
};
class SerialPABotBase_RightJoycon final : public SerialPABotBase_Joycon<RightJoycon>{
public:
    SerialPABotBase_RightJoycon(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerResetMode reset_mode
    );
    ~SerialPABotBase_RightJoycon();
};




}
}
#endif
