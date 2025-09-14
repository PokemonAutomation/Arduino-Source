/*  Nintendo Switch Controller (With Scheduler)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This implements most of the SwitchController API using only the controller
 *  state function. It uses SuperscalarScheduler to do this.
 *
 */

#include "NintendoSwitch_ControllerWithScheduler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;



void ControllerWithScheduler::issue_buttons(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    Button button
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            ButtonFlagType mask = (ButtonFlagType)1 << c;
            if (button & mask){
                m_scheduler.issue_wait_for_resource(schedule, c);
            }
        }
        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            ButtonFlagType mask = (ButtonFlagType)1 << c;
            if (button & mask){
                m_scheduler.issue_to_resource(
                    schedule,
                    std::make_unique<SwitchCommand_Button>((SwitchResource)c),
                    WallDuration::zero(), hold, cooldown
                );
            }
        }
        m_scheduler.issue_nop(schedule, delay);
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        m_logger.log(
            "issue_buttons(): " + button_to_string(button) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}
void ControllerWithScheduler::issue_dpad(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    DpadPosition position
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_Dpad>(position),
            delay, hold, cooldown
        );
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        m_logger.log(
            "issue_dpad(): " + dpad_to_string(position) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}
void ControllerWithScheduler::issue_left_joystick(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    uint8_t x, uint8_t y
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_LeftJoystick>(x, y),
            delay, hold, cooldown
        );
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        m_logger.log(
            "issue_left_joystick(): (" + std::to_string(x) + "," + std::to_string(y) + ")" +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}
void ControllerWithScheduler::issue_right_joystick(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    uint8_t x, uint8_t y
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_RightJoystick>(x, y),
            delay, hold, cooldown
        );
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        m_logger.log(
            "issue_right_joystick(): (" + std::to_string(x) + "," + std::to_string(y) + ")" +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}



void ControllerWithScheduler::issue_gyro(
    const Cancellable* cancellable,
    SwitchResource id, const char* name,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    int16_t value
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_Gyro>(id, value),
            delay, hold, cooldown
        );
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        m_logger.log(
            std::string(name) + "(): (" + std::to_string(value) + ")" +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}



void ControllerWithScheduler::issue_full_controller_state(
    const Cancellable* cancellable,
    Milliseconds hold,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y
){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        if (cancellable){
            cancellable->throw_if_cancelled();
        }

#if 0
        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            ButtonFlagType mask = (ButtonFlagType)1 << c;
            if (button & mask){
                m_scheduler.issue_wait_for_resource(schedule, (size_t)c);
            }
        }
        m_scheduler.issue_wait_for_resource(schedule, (size_t)SwitchResource::DPAD);
        m_scheduler.issue_wait_for_resource(schedule, (size_t)SwitchResource::JOYSTICK_LEFT);
        m_scheduler.issue_wait_for_resource(schedule, (size_t)SwitchResource::JOYSTICK_RIGHT);
#else
        m_scheduler.issue_wait_for_all(schedule);
#endif

        for (size_t c = 0; c < TOTAL_BUTTONS; c++){
            ButtonFlagType mask = (ButtonFlagType)1 << c;
            if (button & mask){
                m_scheduler.issue_to_resource(
                    schedule,
                    std::make_unique<SwitchCommand_Button>((SwitchResource)c),
                    WallDuration::zero(), hold, WallDuration::zero()
                );
            }
        }
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_Dpad>(position),
            WallDuration::zero(), hold, WallDuration::zero()
        );
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_LeftJoystick>(left_x, left_y),
            WallDuration::zero(), hold, WallDuration::zero()
        );
        m_scheduler.issue_to_resource(
            schedule,
            std::make_unique<SwitchCommand_RightJoystick>(right_x, right_y),
            hold, hold, WallDuration::zero()
        );
    }
    execute_schedule(cancellable, schedule);
    if (m_logging_throttler){
        //  Do not log the contents of the command due to privacy concerns.
        //  (people entering passwords)
#if 0
        m_logger.log(
            "issue_controller_state(): (" + button_to_string(button) +
            "), dpad(" + dpad_to_string(position) +
            "), LJ(" + std::to_string(left_x) + "," + std::to_string(left_y) +
            "), RJ(" + std::to_string(right_x) + "," + std::to_string(right_y) +
            "), hold = " + std::to_string(hold.count()) + "ms",
            COLOR_DARKGREEN
        );
#else
        m_logger.log(
            "issue_controller_state(): hold = " + std::to_string(hold.count()) + "ms",
            COLOR_DARKGREEN
        );
#endif
    }
}


void ControllerWithScheduler::issue_mash_button(
    const Cancellable* cancellable,
    Milliseconds duration,
    Button button
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    ThrottleScope scope(m_logging_throttler);
    bool log = true;
    while (duration > Milliseconds::zero()){
        issue_buttons(cancellable, 8*8ms, 5*8ms, 3*8ms, button);

        //  We never log before the first issue to avoid delaying the critical path.
        //  But we do want to log before the mash spam. So we log after the first
        //  issue, but before the second.
        if (log && scope){
            m_logger.log(
                "issue_mash_button(): " + button_to_string(button) +
                ", duration = " + std::to_string(duration.count()) + "ms",
                COLOR_DARKGREEN
            );
        }
        log = false;

        duration = duration >= 8*8ms
            ? duration - 8*8ms
            : Milliseconds::zero();
    }
}
void ControllerWithScheduler::issue_mash_button(
    const Cancellable* cancellable,
    Milliseconds duration,
    Button button0, Button button1
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    ThrottleScope scope(m_logging_throttler);
    bool log = true;
    while (duration > Milliseconds::zero()){
        issue_buttons(cancellable, 4*8ms, 5*8ms, 3*8ms, button0);
        issue_buttons(cancellable, 4*8ms, 5*8ms, 3*8ms, button1);

        //  We never log before the first issue to avoid delaying the critical path.
        //  But we do want to log before the mash spam. So we log after the first
        //  issue, but before the second.
        if (log && scope){
            m_logger.log(
                "issue_mash_button(): (" + button_to_string(button0) +
                "), (" + button_to_string(button1) +
                "), duration = " + std::to_string(duration.count()) + "ms",
                COLOR_DARKGREEN
            );
        }
        log = false;

        duration -= std::min(8*8ms, duration);
    }
}
void ControllerWithScheduler::issue_mash_AZs(
    const Cancellable* cancellable,
    Milliseconds duration
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    ThrottleScope scope(m_logging_throttler);
    bool log = true;
    while (true){
        if (duration <= Milliseconds::zero()){
            break;
        }
        issue_buttons(cancellable, 3*8ms, 6*8ms, 3*8ms, BUTTON_A);

        //  We never log before the first issue to avoid delaying the critical path.
        //  But we do want to log before the mash spam. So we log after the first
        //  issue, but before the second.
        if (log && scope){
            m_logger.log(
                "issue_mash_AZs(): duration = " + std::to_string(duration.count()) + "ms",
                COLOR_DARKGREEN
            );
        }
        log = false;

        duration -= std::min(3*8ms, duration);
        if (duration <= Milliseconds::zero()){
            break;
        }
        issue_buttons(cancellable, 3*8ms, 6*8ms, 3*8ms, BUTTON_ZL);
        duration -= std::min(3*8ms, duration);

        if (duration <= Milliseconds::zero()){
            break;
        }
        issue_buttons(cancellable, 3*8ms, 6*8ms, 3*8ms, BUTTON_ZR);
        duration -= std::min(3*8ms, duration);
    }
}
void ControllerWithScheduler::issue_system_scroll(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    DpadPosition direction  //  Diagonals not allowed.
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    ThrottleScope scope(m_logging_throttler);

    WallClock dpad = m_scheduler.busy_until((size_t)SwitchResource::DPAD);
    WallClock left_joystick = m_scheduler.busy_until((size_t)SwitchResource::JOYSTICK_LEFT);
    WallClock right_joystick = m_scheduler.busy_until((size_t)SwitchResource::JOYSTICK_RIGHT);

    do{
        if (dpad <= left_joystick && dpad <= right_joystick){
            issue_dpad(
                cancellable,
                delay, hold, cooldown,
                direction
            );
            break;
        }

        uint8_t x = 128;
        uint8_t y = 128;
        switch (direction){
        case DPAD_NONE:
            x = 128;
            y = 128;
            break;
        case DPAD_UP:
            x = 128;
            y = 0;
            break;
        case DPAD_RIGHT:
            x = 255;
            y = 128;
            break;
        case DPAD_DOWN:
            x = 128;
            y = 255;
            break;
        case DPAD_LEFT:
            x = 0;
            y = 128;
            break;

        //  These diagonal ones probably don't work.
        case DPAD_UP_RIGHT:
            x = 255;
            y = 0;
            break;
        case DPAD_DOWN_RIGHT:
            x = 255;
            y = 255;
            break;
        case DPAD_DOWN_LEFT:
            x = 0;
            y = 255;
            break;
        case DPAD_UP_LEFT:
            x = 0;
            y = 0;
            break;
        }

        if (left_joystick <= dpad && left_joystick <= right_joystick){
            issue_left_joystick(cancellable, delay, hold, cooldown, x, y);
        }else{
            issue_right_joystick(cancellable, delay, hold, cooldown, x, y);
        }
    }while (false);

    if (scope){
        m_logger.log(
            "issue_system_scroll(): " + dpad_to_string(direction) +
            ", delay = " + std::to_string(delay.count()) + "ms" +
            ", hold = " + std::to_string(hold.count()) + "ms" +
            ", cooldown = " + std::to_string(cooldown.count()) + "ms",
            COLOR_DARKGREEN
        );
    }

}





}
}
