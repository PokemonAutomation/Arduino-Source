/*  Nintendo Switch Pro Controller (SysbotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H
#define PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H

#include "Common/Cpp/Containers/CircularBuffer.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerWithScheduler.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProControllerState.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "SysbotBase_Connection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class ProController_SysbotBase final :
    public ProController,
    public ControllerWithScheduler
{
public:
    using ContextType = ProControllerContext;

    static constexpr size_t QUEUE_SIZE = 4;


public:
    ProController_SysbotBase(
        Logger& logger,
        SysbotBase::TcpSysbotBase_Connection& connection
    );
    ~ProController_SysbotBase();
    void stop();


public:
//    virtual ControllerType controller_type() const override{
//        return ControllerType::NintendoSwitch_WiredController;
//    }

    virtual ControllerPerformanceClass performance_class() const override{
        return ControllerPerformanceClass::SysbotBase;
    }
    virtual Milliseconds ticksize() const override{
        return Milliseconds::zero();
    }
    virtual Milliseconds cooldown() const override{
        return Milliseconds(150);
    }
    virtual Milliseconds timing_variation() const override{
        return ConsoleSettings::instance().TIMING_OPTIONS.SYSBOTBASE;
    }
    virtual bool atomic_multibutton() const override{
        return false;
    }


public:
    virtual Logger& logger() override{
        return m_logger;
    }
    virtual RecursiveThrottler& logging_throttler() override{
        return m_logging_throttler;
    }
    virtual bool is_ready() const override{
        return m_connection.is_ready();
    }


public:
    virtual void cancel_all_commands() override;
    virtual void replace_on_next_command() override;

    virtual void wait_for_all(Cancellable* cancellable) override;


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(Cancellable* cancellable) override{
        ControllerWithScheduler::issue_barrier(cancellable);
    }
    virtual void issue_nop(Cancellable* cancellable, Milliseconds duration) override{
        ControllerWithScheduler::issue_nop(cancellable, duration);
    }

    virtual void issue_buttons(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    ) override{
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
    }
    virtual void issue_dpad(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    ) override{
        ControllerWithScheduler::issue_dpad(cancellable, delay, hold, cooldown, position);
    }
    virtual void issue_left_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    ) override{
        ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, position);
    }
    virtual void issue_right_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    ) override{
        ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, position);
    }

    virtual void issue_gyro_accel_x(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_x(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_accel_y(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_y(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_accel_z(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_accel_z(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_x(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_x(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_y(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_y(cancellable, delay, hold, cooldown, value);
    }
    virtual void issue_gyro_rotate_z(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ) override{
        ControllerWithScheduler::issue_gyro_rotate_z(cancellable, delay, hold, cooldown, value);
    }

    virtual void issue_full_controller_state(
        Cancellable* cancellable,
        bool enable_logging,
        Milliseconds duration,
        Button button,
        DpadPosition dpad,
        const JoystickPosition& left_joystick,
        const JoystickPosition& right_joystick
    ) override{
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            enable_logging,
            duration,
            button,
            dpad,
            left_joystick,
            right_joystick
        );
    }


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button, delay, hold, cooldown);
    }
    virtual void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        Button button0, Button button1
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button0, button1);
    }
    virtual void issue_mash_AZs(
        Cancellable* cancellable,
        Milliseconds duration
    ) override{
        ControllerWithScheduler::issue_mash_AZs(cancellable, duration);
    }
    virtual void issue_system_scroll(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    ) override{
        ControllerWithScheduler::issue_system_scroll(cancellable, delay, hold, cooldown, direction);
    }


private:
    virtual void execute_state(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override;

    void send_diff(
        const ProControllerState& old_state,
        const ProControllerState& new_state
    );
    void thread_body();


private:
    SysbotBase::TcpSysbotBase_Connection& m_connection;

    std::atomic<bool> m_stopping;
    bool m_replace_on_next;

    struct Command{
        ProControllerState state;
        Milliseconds duration;
    };
    CircularBuffer<Command> m_command_queue;

    //  WallClock::max() means the queue is empty.
    //  WallClock::min() means the state has suddently changed.
    WallClock m_next_state_change;

    ConditionVariable m_cv;
    std::unique_ptr<AsyncTask> m_dispatch_thread;
};




}
}
#endif
