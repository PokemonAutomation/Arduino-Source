/*  Nintendo Switch Controller (With Scheduler)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This implements most of the SwitchController API using only the controller
 *  state function. It uses SuperscalarScheduler to do this.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerWithScheduler_H
#define PokemonAutomation_NintendoSwitch_ControllerWithScheduler_H

#include <mutex>
#include "Common/Cpp/RecursiveThrottler.h"
#include "Controllers/SuperscalarScheduler.h"
#include "NintendoSwitch_ControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



struct SwitchButton_Dpad : public ExecutionResource{
    DpadPosition position;
};
struct SwitchButton_Joystick : public ExecutionResource{
    uint8_t x;
    uint8_t y;
};
struct SwitchGyro : public ExecutionResource{
    int16_t value;
};
struct ControllerSchedulerState{
    ExecutionResource m_buttons[TOTAL_BUTTONS];
    SwitchButton_Dpad m_dpad;
    SwitchButton_Joystick m_left_joystick;
    SwitchButton_Joystick m_right_joystick;

    SwitchGyro m_accel_x;
    SwitchGyro m_accel_y;
    SwitchGyro m_accel_z;
    SwitchGyro m_rotation_x;
    SwitchGyro m_rotation_y;
    SwitchGyro m_rotation_z;

    bool is_active() const{
        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            if (m_buttons[c].is_busy()){
                return true;
            }
        }
        if (m_dpad.is_busy()) return true;
        if (m_left_joystick.is_busy()) return true;
        if (m_right_joystick.is_busy()) return true;

        if (m_accel_x.is_busy()) return true;
        if (m_accel_y.is_busy()) return true;
        if (m_accel_z.is_busy()) return true;
        if (m_rotation_x.is_busy()) return true;
        if (m_rotation_y.is_busy()) return true;
        if (m_rotation_z.is_busy()) return true;

        return false;
    }

    std::vector<ExecutionResource*> make_resource_list(){
        std::vector<ExecutionResource*> ret;
        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            ret.emplace_back(m_buttons + c);
        }
        ret.emplace_back(&m_dpad);
        ret.emplace_back(&m_left_joystick);
        ret.emplace_back(&m_right_joystick);
        ret.emplace_back(&m_accel_x);
        ret.emplace_back(&m_accel_y);
        ret.emplace_back(&m_accel_z);
        ret.emplace_back(&m_rotation_x);
        ret.emplace_back(&m_rotation_y);
        ret.emplace_back(&m_rotation_z);
        return ret;
    }
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





class ControllerWithScheduler :
    protected ControllerSchedulerState,
    protected SuperscalarScheduler
{
public:
    ControllerWithScheduler(Logger& logger);

    RecursiveThrottler& logging_throttler(){
        return m_logging_throttler;
    }


public:
    //  Superscalar Commands (the "ssf" framework)

    void issue_barrier(const Cancellable* cancellable);
    void issue_nop(const Cancellable* cancellable, Milliseconds duration);
    void issue_buttons(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        Button button
    );
    void issue_dpad(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition position
    );
    void issue_left_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    );
    void issue_right_joystick(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        uint8_t x, uint8_t y
    );

    void issue_gyro(
        const Cancellable* cancellable,
        SwitchGyro& gyro, const char* name,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    );
    void issue_gyro_accel_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, m_accel_x, "issue_gyro_accel_x", delay, hold, cooldown, value);
    }
    void issue_gyro_accel_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, m_accel_y, "issue_gyro_accel_y", delay, hold, cooldown, value);
    }
    void issue_gyro_accel_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, m_accel_z, "issue_gyro_accel_z", delay, hold, cooldown, value);
    }
    void issue_gyro_rotate_x(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, m_rotation_x, "issue_gyro_rotate_x", delay, hold, cooldown, value);
    }
    void issue_gyro_rotate_y(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, m_rotation_y, "issue_gyro_rotate_y", delay, hold, cooldown, value);
    }
    void issue_gyro_rotate_z(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        int16_t value
    ){
        issue_gyro(cancellable, m_rotation_z, "issue_gyro_rotate_z", delay, hold, cooldown, value);
    }

    void issue_full_controller_state(
        const Cancellable* cancellable,
        Milliseconds hold,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y
    );


public:
    //  High speed RPCs.

    void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button
    );
    void issue_mash_button(
        const Cancellable* cancellable,
        Milliseconds duration,
        Button button0, Button button1
    );
    void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    );
    void issue_system_scroll(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        DpadPosition direction  //  Diagonals not allowed.
    );


protected:
#if 0
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
#endif

//    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;


protected:
    Logger& m_logger;
//    std::atomic<size_t> m_logging_suppress;
    RecursiveThrottler m_logging_throttler;

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
