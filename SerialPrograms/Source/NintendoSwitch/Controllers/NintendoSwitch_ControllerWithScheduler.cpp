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




ControllerWithScheduler::ControllerWithScheduler(Logger& logger)
    : SuperscalarScheduler(
        logger, Milliseconds(4),
        make_resource_list()
    )
    , m_logger(logger)
//    , m_logging_suppress(0)
{}



void ControllerWithScheduler::issue_barrier(const Cancellable* cancellable){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    this->issue_wait_for_all(cancellable);
    if (m_logging_throttler){
        m_logger.log("issue_barrier()", COLOR_DARKGREEN);
    }
}
void ControllerWithScheduler::issue_nop(const Cancellable* cancellable, Milliseconds duration){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    this->SuperscalarScheduler::issue_nop(cancellable, WallDuration(duration));
    if (m_logging_throttler){
        m_logger.log(
            "issue_nop(): duration = " + std::to_string(duration.count()) + "ms",
            COLOR_DARKGREEN
        );
    }
}
void ControllerWithScheduler::issue_buttons(
    const Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    Button button
){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        ButtonFlagType mask = (ButtonFlagType)1 << c;
        if (button & mask){
            this->issue_wait_for_resource(cancellable, m_buttons[c]);
        }
    }
    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        ButtonFlagType mask = (ButtonFlagType)1 << c;
        if (button & mask){
            this->issue_to_resource(
                cancellable, m_buttons[c],
                WallDuration::zero(), hold, cooldown
            );
        }
    }
    this->SuperscalarScheduler::issue_nop(cancellable, delay);

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
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    this->issue_wait_for_resource(cancellable, m_dpad);
    m_dpad.position = position;
    this->issue_to_resource(cancellable, m_dpad, delay, hold, cooldown);

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
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);

    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    this->issue_wait_for_resource(cancellable, m_left_joystick);
    m_left_joystick.x = x;
    m_left_joystick.y = y;
    this->issue_to_resource(cancellable, m_left_joystick, delay, hold, cooldown);

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
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    this->issue_wait_for_resource(cancellable, m_right_joystick);
    m_right_joystick.x = x;
    m_right_joystick.y = y;
    this->issue_to_resource(cancellable, m_right_joystick, delay, hold, cooldown);

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
    SwitchGyro& gyro, const char* name,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    int16_t value
){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    this->issue_wait_for_resource(cancellable, gyro);
    gyro.value = value;
    this->issue_to_resource(cancellable, gyro, delay, hold, cooldown);

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
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::lock_guard<std::mutex> lg1(m_state_lock);
    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        ButtonFlagType mask = (ButtonFlagType)1 << c;
        if (button & mask){
            this->issue_wait_for_resource(cancellable, m_buttons[c]);
        }
    }
    this->issue_wait_for_resource(cancellable, m_dpad);
    this->issue_wait_for_resource(cancellable, m_left_joystick);
    this->issue_wait_for_resource(cancellable, m_right_joystick);

    m_dpad.position = position;
    m_left_joystick.x = left_x;
    m_left_joystick.y = left_y;
    m_right_joystick.x = right_x;
    m_right_joystick.y = right_y;

    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        ButtonFlagType mask = (ButtonFlagType)1 << c;
        if (button & mask){
            this->issue_to_resource(
                cancellable, m_buttons[c],
                WallDuration::zero(), hold, WallDuration::zero()
            );
        }
    }
    this->issue_to_resource(
        cancellable, m_dpad,
        WallDuration::zero(), hold, WallDuration::zero()
    );
    this->issue_to_resource(
        cancellable, m_left_joystick,
        WallDuration::zero(), hold, WallDuration::zero()
    );
    this->issue_to_resource(
        cancellable, m_right_joystick,
        hold, hold, WallDuration::zero()
    );

    if (m_logging_throttler){
        m_logger.log(
            "issue_controller_state(): (" + button_to_string(button) +
            "), dpad(" + dpad_to_string(position) +
            "), LJ(" + std::to_string(left_x) + "," + std::to_string(left_y) +
            "), RJ(" + std::to_string(right_x) + "," + std::to_string(right_y) +
            "), hold = " + std::to_string(hold.count()) + "ms",
            COLOR_DARKGREEN
        );
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

    WallClock dpad = m_dpad.free_time();
    WallClock left_joystick = m_left_joystick.free_time();
    WallClock right_joystick = m_right_joystick.free_time();

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
