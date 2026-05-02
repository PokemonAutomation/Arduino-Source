/*  PABotBase2: Joycon (Nintendo Switch 1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PABotBase2_Joycon_H
#define PokemonAutomation_NintendoSwitch_PABotBase2_Joycon_H

#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "NintendoSwitch_PABotBase2_OemController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




template <typename JoyconType>
class PABotBase2_JoyCon :
    public JoyconType,
    public PABotBase2_OemController
{
    static constexpr uint16_t JOYSTICK_MIN_THRESHOLD = 1874;
    static constexpr uint16_t JOYSTICK_MAX_THRESHOLD = 260;

public:
    PABotBase2_JoyCon(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerClass controller_class,
        ControllerType controller_type,
        std::function<void(double magnitude)> on_rumble
    )
        : JoyconType(logger, controller_class)
        , PABotBase2_OemController(
            logger, connection, controller_type,
            std::move(on_rumble)
        )
        , m_controller_type(controller_type)
    {}
    ~PABotBase2_JoyCon(){
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
        button &= m_valid_buttons;
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
    }
    virtual void issue_joystick(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const JoystickPosition& position
    ) override{
        switch (this->controller_class()){
        case ControllerClass::NintendoSwitch_LeftJoycon:
            ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, position);
            break;
        case ControllerClass::NintendoSwitch_RightJoycon:
            ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, position);
            break;
        default:
            throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
        }
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
        const JoystickPosition& joystick
    ) override{
        button &= m_valid_buttons;
        switch (this->controller_class()){
        case ControllerClass::NintendoSwitch_LeftJoycon:
            ControllerWithScheduler::issue_full_controller_state(
                cancellable,
                enable_logging,
                duration,
                button,
                DPAD_NONE,
                joystick,
                {0, 0}
            );
            break;
        case ControllerClass::NintendoSwitch_RightJoycon:
            ControllerWithScheduler::issue_full_controller_state(
                cancellable,
                enable_logging,
                duration,
                button,
                DPAD_NONE,
                {0, 0},
                joystick
            );
            break;
        default:;
        }
    }


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        Cancellable* cancellable,
        Button button, Milliseconds duration,
        Milliseconds delay,
        Milliseconds hold,
        Milliseconds cooldown
    ) override{
        button &= m_valid_buttons;
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button, delay, hold, cooldown);
    }


protected:
    void execute_state_left_joycon(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ){
        SwitchControllerState controller_state;
        for (auto& item : entry.state){
            static_cast<const SwitchCommand&>(*item).apply(controller_state);
        }

        pabb_NintendoSwitch_OemController_State0x30_Buttons buttons{
            .button3 = 0,
            .button4 = 0,
            .button5 = 0,
            .left_joystick = {0x00, 0x08, 0x80},
            .right_joystick = {0x00, 0x08, 0x80},
            .vibrator = 0x00,
        };

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

        pabb_NintendoSwitch_OemController_State0x30_Gyro gyro{};
        bool gyro_active = populate_report_gyro(gyro, controller_state);

        if (!gyro_active){
            issue_report(cancellable, entry.duration, buttons);
        }else{
            issue_report(cancellable, entry.duration, buttons, gyro);
        }
    }
    void execute_state_right_joycon(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ){
        SwitchControllerState controller_state;
        for (auto& item : entry.state){
            static_cast<const SwitchCommand&>(*item).apply(controller_state);
        }

        pabb_NintendoSwitch_OemController_State0x30_Buttons buttons{
            .button3 = 0,
            .button4 = 0,
            .button5 = 0,
            .left_joystick = {0x00, 0x08, 0x80},
            .right_joystick = {0x00, 0x08, 0x80},
            .vibrator = 0x00,
        };

        populate_report_buttons(buttons, controller_state);

        //  Right Stick
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.right_joystick,
            controller_state.right_joystick
        );

    #if 0
        cout << (int)controller_state.right_stick_x << " - "
             << (int)controller_state.right_stick_y << ": "
             << std::chrono::duration_cast<Milliseconds>(entry.duration).count() << endl;
    #endif

        pabb_NintendoSwitch_OemController_State0x30_Gyro gyro{};
        bool gyro_active = populate_report_gyro(gyro, controller_state);

        if (!gyro_active){
            issue_report(cancellable, entry.duration, buttons);
        }else{
            issue_report(cancellable, entry.duration, buttons, gyro);
        }
    }
    virtual void execute_state(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override{
        switch (this->controller_class()){
        case ControllerClass::NintendoSwitch_LeftJoycon:
            execute_state_left_joycon(cancellable, entry);
            break;
        case ControllerClass::NintendoSwitch_RightJoycon:
            execute_state_right_joycon(cancellable, entry);
            break;
        default:
            throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
        }
    }

    const ControllerType m_controller_type;
    Button m_valid_buttons;
};



class PABotBase2_LeftJoycon final : public PABotBase2_JoyCon<LeftJoycon>{
public:
    PABotBase2_LeftJoycon(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type
    )
        : PABotBase2_JoyCon<LeftJoycon>(
            logger, connection,
            ControllerClass::NintendoSwitch_LeftJoycon,
            controller_type,
            [this](double magnitude){ on_rumble(magnitude); }
        )
    {
        m_valid_buttons = VALID_LEFT_JOYCON_BUTTONS;
    }
    ~PABotBase2_LeftJoycon(){
        PABotBase2_OemController::stop();
    }
};
class PABotBase2_RightJoycon final : public PABotBase2_JoyCon<RightJoycon>{
public:
    PABotBase2_RightJoycon(
        Logger& logger,
        PABotBase2::Connection& connection,
        ControllerType controller_type
    )
        : PABotBase2_JoyCon<RightJoycon>(
            logger, connection,
            ControllerClass::NintendoSwitch_RightJoycon,
            controller_type,
            [this](double magnitude){ on_rumble(magnitude); }
        )
    {
        m_valid_buttons = VALID_RIGHT_JOYCON_BUTTONS;
    }
    ~PABotBase2_RightJoycon(){
        PABotBase2_OemController::stop();
    }
};



}
}
#endif
