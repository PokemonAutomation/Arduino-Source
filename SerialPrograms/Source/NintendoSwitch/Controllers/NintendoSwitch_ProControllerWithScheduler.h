/*  Nintendo Switch Controller (With Scheduler)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This implements most of the SwitchController API using only the controller
 *  state function. It uses SuperscalarScheduler to do this.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerWithScheduler_H
#define PokemonAutomation_NintendoSwitch_ControllerWithScheduler_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Controllers/SuperscalarScheduler.h"
#include "NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{






struct SwitchButton_Dpad : public ExecutionResource{
    DpadPosition position;
};
struct SwitchButton_Joystick : public ExecutionResource{
    uint8_t x;
    uint8_t y;
};
struct ProControllerSchedulerState{
    ExecutionResource m_buttons[14];
    SwitchButton_Dpad m_dpad;
    SwitchButton_Joystick m_left_joystick;
    SwitchButton_Joystick m_right_joystick;
};






class ProControllerWithScheduler :
    public ProController,
    private ProControllerSchedulerState,
    protected SuperscalarScheduler
{
public:
    ProControllerWithScheduler(Logger& logger);


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(const Cancellable* cancellable) override;
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) override;
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_dpad(
        const Cancellable* cancellable,
        DpadPosition position,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button, Milliseconds duration
    ) override;
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, Milliseconds duration
    ) override;
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) override;
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;


private:
    class LoggingSuppressScope{
    public:
        LoggingSuppressScope(std::atomic<size_t>& counter)
            : m_counter(counter)
        {
            m_counter++;
        }
        ~LoggingSuppressScope(){
            m_counter--;
        }
    private:
        std::atomic<size_t>& m_counter;
    };

    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;


protected:
    Logger& m_logger;
    std::atomic<size_t> m_logging_suppress;
    SpinLock m_lock;
};




}
}
#endif
