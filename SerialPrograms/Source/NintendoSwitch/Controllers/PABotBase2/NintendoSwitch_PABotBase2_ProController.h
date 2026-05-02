/*  PABotBase2: Pro Controller (Nintendo Switch 1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PABotBase2_ProController_H
#define PokemonAutomation_NintendoSwitch_PABotBase2_ProController_H

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch_PABotBase2_OemController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class PABotBase2_ProController final :
    public ProController,
    public PABotBase2_OemController
{
    static constexpr uint16_t JOYSTICK_MIN_THRESHOLD = 1874;
    static constexpr uint16_t JOYSTICK_MAX_THRESHOLD = 320;

public:
    PABotBase2_ProController(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type
    )
        : ProController(logger)
        , PABotBase2_OemController(
            logger, connection, controller_type,
            [this](double magnitude){ on_rumble(magnitude); }
        )
    {}
    ~PABotBase2_ProController(){
        PABotBase2_OemController::stop();
    }

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual RecursiveThrottler& logging_throttler() override{
        return m_logging_throttler;
    }
    virtual bool is_ready() const override{
        return PABotBase2_Controller::is_ready();
    }


public:
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
        return PABotBase2_Controller::cancel_all_commands();
    }
    virtual void replace_on_next_command() override{
        PABotBase2_Controller::replace_on_next_command();
    }

    virtual void wait_for_all(Cancellable* cancellable) override{
        PABotBase2_Controller::wait_for_all(cancellable);
    }


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
    ) override{
        SwitchControllerState controller_state;
        for (auto& item : entry.state){
            static_cast<const SwitchCommand&>(*item).apply(controller_state);
        }

        //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md
        pabb_NintendoSwitch_OemController_State0x30_Buttons buttons{
            .button3 = 0,
            .button4 = 0,
            .button5 = 0,
            .left_joystick = {0x00, 0x08, 0x80},
            .right_joystick = {0x00, 0x08, 0x80},
            .vibrator = 0x00,
        };

    //    Button all_buttons =
        populate_report_buttons(buttons, controller_state);

        {
            SplitDpad dpad = convert_unified_to_split_dpad(controller_state.dpad);
            buttons.button5 |= (dpad.down  ? 1 : 0) << 0;
            buttons.button5 |= (dpad.up    ? 1 : 0) << 1;
            buttons.button5 |= (dpad.right ? 1 : 0) << 2;
            buttons.button5 |= (dpad.left  ? 1 : 0) << 3;
        }

        //  Left Stick
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.left_joystick,
            controller_state.left_joystick
        );

        //  Right Stick
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.right_joystick,
            controller_state.right_joystick
        );

        pabb_NintendoSwitch_OemController_State0x30_Gyro gyro{
            0x0000,
            0x0000,
            0x0000,
            0x0000,
            0x0000,
            0x0000,
        };
        bool gyro_active = populate_report_gyro(gyro, controller_state);

    //    gyro_active = true;
    //    gyro.rotation_y = 0x00ff;
    //    gyro.rotation_z = 0x000f;

        if (!gyro_active){
            issue_report(cancellable, entry.duration, buttons);
        }else{
            issue_report(cancellable, entry.duration, buttons, gyro);
        }

    #if 0
        m_logger.log(
            "push_state(): (" + button_to_string(all_buttons) +
            "), dpad(" + dpad_to_string(m_dpad.position) +
            "), LJ(" + std::to_string(m_left_joystick.x) + "," + std::to_string(m_left_joystick.y) +
            "), RJ(" + std::to_string(m_right_joystick.x) + "," + std::to_string(m_right_joystick.y) +
            "), hold = " + std::to_string(std::chrono::duration_cast<Milliseconds>(duration).count()) + "ms",
            COLOR_DARKGREEN
        );
    #endif
    }
};



}
}
#endif
