/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Controller_H
#define PokemonAutomation_NintendoSwitch_Controller_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class SwitchControllerContext;



inline std::string button_to_string(Button button){
    std::string str;
    if (button & BUTTON_Y) str += "Y ";
    if (button & BUTTON_B) str += "B ";
    if (button & BUTTON_A) str += "A ";
    if (button & BUTTON_X) str += "X ";
    if (button & BUTTON_L) str += "L ";
    if (button & BUTTON_R) str += "R ";
    if (button & BUTTON_ZL) str += "ZL ";
    if (button & BUTTON_ZR) str += "ZR ";
    if (button & BUTTON_MINUS) str += "- ";
    if (button & BUTTON_PLUS) str += "+ ";
    if (button & BUTTON_LCLICK) str += "LJ ";
    if (button & BUTTON_RCLICK) str += "RJ ";
    if (button & BUTTON_HOME) str += "HOME ";
    if (button & BUTTON_CAPTURE) str += "CAPTURE ";
    if (str.back() == ' '){
        str.pop_back();
    }
    if (str.empty()){
        str = "none";
    }
    return str;
}
inline std::string dpad_to_string(DpadPosition dpad){
    switch (dpad){
    case DPAD_UP            : return "up";
    case DPAD_UP_RIGHT      : return "up-right";
    case DPAD_RIGHT         : return "right";
    case DPAD_DOWN_RIGHT    : return "down-right";
    case DPAD_DOWN          : return "down";
    case DPAD_DOWN_LEFT     : return "down-left";
    case DPAD_LEFT          : return "left";
    case DPAD_UP_LEFT       : return "up-left";
    case DPAD_NONE          : return "none";
    }
    return "unknown";
}




//
//  This is the generic interface a Switch controller that encapsulates all
//  Switch controllers.
//
//  Implementations must inherit from both this class ControllerConnection as
//  the framework will cross-cast from ControllerConnection to SwitchController.
//
//  Currently we only have one implementation (SerialPABotBase). But we expect
//  to add more in the future.
//
class SwitchController{
public:
    using ContextType = SwitchControllerContext;

    virtual Logger& logger() = 0;


public:
    //  General Control

    //  Wait for all unfinished commands to finish.
    virtual void wait_for_all(const Cancellable* cancellable) = 0;

    //  Cancel all commands. This returns the controller to the neutral button
    //  state and clears the command queue.
    virtual void cancel_all(const Cancellable* cancellable) = 0;

    //  Declare that the next command will replace the current command stream
    //  with no gaps.
    virtual void replace_on_next_command(const Cancellable* cancellable) = 0;


public:
    //  Basic Commands

    //  Required Feature Flags:
    //    - NintendoSwitch_Basic

    //
    //  All commands are enqueued into a FIFO that the controller will execute
    //  in order preserving the timing semantics as closely as possible
    //  irrespective of the latencies between the host and the device.
    //
    //  For wired controller emulation, the timings will be preserved exactly as
    //  long as the FIFO never completely empties out.
    //
    //  For wireless controllers (Joy Con), while we do not have an
    //  implementation of this at this time, we do not expect it to be able to
    //  preserve timing.
    //
    //  Whether a controller supports exact timings is controlled by the feature
    //  flag "TickPrecise".
    //

    //  The following functions are asynchronous. They will return immediately
    //  if the command can be enqueued into the FIFO. Otherwise, they will block
    //  until there is space in the FIFO.

    //
    //  Press all the following buttons/joysticks simultaneously for the
    //  specified duration. No wait is added at the end. Thus you can issue
    //  these back-to-back to simulate buttons being pressed and released
    //  concurrently with other buttons being held down the whole time.
    //
    //  The behavior of this function is undefined if there are any unfinished
    //  asynchronous commands in the controller's queue (including unfinished
    //  cooldowns). It is the responsibility of the caller to ensure the
    //  controller is idle by calling "this->wait_for_all_requests()".
    //
    //  The sole purpose of this function is for keyboard commands.
    //  While it's technically possible to implement any button overlapping
    //  sequence with this, doing so this way can lead to very inefficient
    //  serial bandwidth usage if buttons are being rapidly pressed and released
    //  in an arbitrary manner that leads to constant state changes.
    //
    //  If we need to support new Switch controller functionality
    //  (such as Joycon gyro or new stuff in Switch 2), we can simply add
    //  overloads to this and gate them behind features.
    //
    virtual void issue_controller_state(
        const Cancellable* cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        Milliseconds milliseconds
    ) = 0;

    //  Temporary for refactor: Send custom requests for PABotBase's advanced
    //  RPCs.
    virtual void send_botbase_request(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) = 0;
    virtual BotBaseMessage send_botbase_request_and_wait(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) = 0;


public:
    //  Superscalar Commands (the "ssf" framework)

    //  Required Feature Flags:
    //    - NintendoSwitch_SSF

    //  Tell the scheduler to wait for all pending commands to finish
    //  (including cooldowns) before executing further instructions.
    //  This is used to prevent hanging commands from overlapping with new
    //  commands issued after this barrier.
    virtual void issue_barrier(const Cancellable* cancellable) = 0;

    //  Do nothing for this many ticks.
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) = 0;

    //
    //  delay       The # of ticks to wait before moving onto the next command.
    //  hold        The # of ticks to hold the button/stick down for.
    //  cooldown    After the button has been released, prevent it from being
    //              used again for this many ticks.
    //
    //  For "normal" use, you should always set (delay == hold + cooldown).
    //  This is the easiest case to understand and is what the "pbf" interface
    //  exposes.
    //
    //  If the button is busy (due to still being held down or is waiting out
    //  the cooldown), the command will block until the button is ready.
    //
    //  By setting (delay < hold), the command will "return" early and move onto
    //  the next command while the button is still being held down.
    //
    //  If a command returns before it is finished (delay < hold + cooldown),
    //  it is considered a "hanging" command.
    //
    //  This allows you to overlap buttons. But is confusing to use because it
    //  implies a non-trivial controller state with pending button presses that
    //  are scheduled to be released at a later time.
    //
    //  Each button/stick has its own independent timeline/schedule.
    //  Users are responsible for understanding the controller state and
    //  managing the timeline/scheduling.
    //

    //  Press all the buttons set in the bitfield simultaneously.
    //  This command will wait until all the selected buttons are ready to
    //  ensure that they are all dispatched simultaneously.
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;

    //  Dpad
    virtual void issue_dpad(
        const Cancellable* cancellable,
        DpadPosition position,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;

    //  Joysticks
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;


public:
    //  High speed RPCs.

    //  Required Feature Flags:
    //    - NintendoSwitch_Macros
    //
    //  Almost everything uses these. So it might be worth moving these to
    //  "NintendoSwitch_Basic" instead.

    //
    //  It is currently unclear if these can be properly executed over wireless.
    //  If they can't, then it remains to be decided if we should gate these
    //  behind a feature flag or if the controller should slow them down to make
    //  them work properly.
    //
    //  It is not advised to call these if you are micromanaging with tick-level
    //  precision since the exact timing characteristics and button selection
    //  is not specified and context-dependent.
    //

    //  Mash a button as quickly as possible.
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button, Milliseconds duration
    ) = 0;

    //  Alternate pressing "button0" and "button1" as quickly as possible.
    //  "button0" will always be pressed first.
    //  Both buttons will be pressed at least once.
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, Milliseconds dutation
    ) = 0;

    //  In situations where A, ZL, and RL all do the same thing, use all 3 of
    //  them to logically mash A much faster than is possible with just one
    //  button.
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) = 0;

    //
    //  Send a scroll command in the specified direction.
    //
    //  This will use either the dpad or either joystick - whichever is
    //  available first in the pipeline.
    //
    //  The intended use-case of this for fast scrolling in the system menu
    //  where all 3 buttons have the same effect and can be used at the same
    //  time.
    //
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) = 0;

};




//
//  The context wrapper to support asynchronous cancel.
//
class SwitchControllerContext final : public CancellableScope{
public:
    using ControllerType = SwitchController;

public:
    SwitchControllerContext(SwitchController& botbase)
        : m_controller(botbase)
    {}
    SwitchControllerContext(CancellableScope& parent, SwitchController& botbase)
        : m_controller(botbase)
    {
        attach(parent);
    }
    virtual ~SwitchControllerContext(){
        detach();
    }

    SwitchController* operator->(){
        m_lifetime_sanitizer.check_usage();
        return &m_controller;
    }

    operator SwitchController&() const{ return m_controller; }
    SwitchController& controller() const{ return m_controller; }

#if 1   //  REMOVE
    void wait_for_all_requests() const{
        m_lifetime_sanitizer.check_usage();
        m_controller.wait_for_all(this);
    }
#endif

    //  Stop all commands in this context now.
    void cancel_now(){
        m_lifetime_sanitizer.check_usage();
        CancellableScope::cancel(nullptr);
        m_controller.cancel_all(this);
    }

    //  Stop the commands in this context, but do it lazily.
    //
    //  1.  Stop new commands from being issued to the device from this context.
    //  2.  Tell the device that the next command that is issued should replace
    //      the command queue.
    //
    //  This cancel is used when you need continuity from an ongoing
    //  sequence.
    void cancel_lazy(){
        m_lifetime_sanitizer.check_usage();
        CancellableScope::cancel(nullptr);
        m_controller.replace_on_next_command(this);
    }

    virtual bool cancel(std::exception_ptr exception) noexcept override{
        m_lifetime_sanitizer.check_usage();
        if (CancellableScope::cancel(std::move(exception))){
            return true;
        }
        try{
            m_controller.cancel_all(this);
        }catch (...){}
        return false;
    }


private:
    SwitchController& m_controller;
    LifetimeSanitizer m_lifetime_sanitizer;
};





}
}
#endif
