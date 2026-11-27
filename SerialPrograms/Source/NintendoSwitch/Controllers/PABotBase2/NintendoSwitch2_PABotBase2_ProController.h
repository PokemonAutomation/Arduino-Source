/*  PABotBase2: Pro Controller (Nintendo Switch 2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_PABotBase2_ProController_H
#define PokemonAutomation_NintendoSwitch2_PABotBase2_ProController_H

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch2_PABotBase2_OemController.h"

namespace PokemonAutomation{
namespace NintendoSwitch2{




class PABotBase2_ProController final :
    public NintendoSwitch::ProController,
    public PABotBase2_OemController
{
    using PABotBase2_Controller = NintendoSwitch::PABotBase2_Controller;

    //  TODO: Exact values TBD
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
            logger, logging_throttler(),
            connection, controller_type,
            [this](double magnitude){ on_rumble(magnitude); }
        )
    {}
    ~PABotBase2_ProController(){
        PABotBase2_OemController::stop();
    }

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual bool is_ready() const override{
        return PABotBase2_Controller::is_ready();
    }
    virtual ControllerPlayerNumber get_player_number(Cancellable& cancellable) override{
        return PABotBase2_OemController::get_player_number(cancellable);
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
    virtual bool cancel_all_and_wait(Milliseconds timeout) override{
        return PABotBase2_Controller::cancel_all_and_wait(timeout);
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
        NintendoSwitch::Button button
    ) override{
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
    }
    virtual void issue_dpad(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        NintendoSwitch::DpadPosition position
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

    virtual void issue_full_controller_state(
        Cancellable* cancellable,
        bool enable_logging,
        Milliseconds duration,
        NintendoSwitch::Button button,
        NintendoSwitch::DpadPosition dpad,
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
        NintendoSwitch::Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override{
        ControllerWithScheduler::issue_mash_button(cancellable, duration, button, delay, hold, cooldown);
    }
    virtual void issue_mash_button(
        Cancellable* cancellable,
        Milliseconds duration,
        NintendoSwitch::Button button0, NintendoSwitch::Button button1
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
        NintendoSwitch::DpadPosition direction  //  Diagonals not allowed.
    ) override{
        ControllerWithScheduler::issue_system_scroll(cancellable, delay, hold, cooldown, direction);
    }


private:
    virtual void execute_state(
        Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override{
        NintendoSwitch::SwitchControllerState controller_state;
        for (auto& item : entry.state){
            static_cast<const NintendoSwitch::SwitchCommand&>(*item).apply(controller_state);
        }



    }






};




}
}
#endif
