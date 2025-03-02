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

#include <mutex>
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


struct SplitDpad{
    bool up = false;
    bool right = false;
    bool down = false;
    bool left = false;
};
inline SplitDpad convert_unified_to_split_dpad(DpadPosition dpad){
    switch (dpad){
    case DpadPosition::DPAD_UP:
        return {true, false, false, false};
    case DpadPosition::DPAD_UP_RIGHT:
        return {true, true, false, false};
    case DpadPosition::DPAD_RIGHT:
        return {false, true, false, false};
    case DpadPosition::DPAD_DOWN_RIGHT:
        return {false, true, true, false};
    case DpadPosition::DPAD_DOWN:
        return {false, false, true, false};
    case DpadPosition::DPAD_DOWN_LEFT:
        return {false, false, true, true};
    case DpadPosition::DPAD_LEFT:
        return {false, false, false, true};
    case DpadPosition::DPAD_UP_LEFT:
        return {true, false, false, true};
    default:
        return {false, false, false, false};
    }
}





class ProControllerWithScheduler :
    public ProController,
    protected ProControllerSchedulerState,
    protected SuperscalarScheduler
{
public:
    ProControllerWithScheduler(Logger& logger, Milliseconds timing_variation);

    virtual Logger& logger() override{
        return m_logger;
    }


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
    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        Milliseconds hold
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


protected:
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

//    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;


protected:
    Logger& m_logger;
    std::atomic<size_t> m_logging_suppress;

    //  If you need both of these locks, always acquire "m_issue_lock" first.

    //  This lock makes sure that only one command is issued at a time. It can
    //  be held for long periods of time if the command queue is full.
    std::mutex m_issue_lock;

    //  This lock protects the state/fields of this class and subclasses.
    //  This lock is never held for a long time.
    std::mutex m_state_lock;
};




}
}
#endif
