/*  Nintendo Switch Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This is the raw (full) controller API that is exposed to programs.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Controller_H
#define PokemonAutomation_NintendoSwitch_Controller_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "Controllers/Controller.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



class ProControllerContext;



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
//  This is the generic interface a Switch pro controller.
//
//  Currently we only have one implementation (SerialPABotBase). But we expect
//  to add more in the future.
//
class ProController : public AbstractController{
public:
    using ContextType = ProControllerContext;

    virtual ~ProController();
    ProController(Milliseconds timing_variation);

    //  Must call before destruction begins.
    void stop() noexcept;

    virtual Logger& logger() = 0;

    Milliseconds timing_variation() const{
        return m_timing_variation;
    }


public:
    //
    //  Cancellation
    //
    //  These functions will return immediately and are thread-safe with
    //  everything. The intended use-case is for an inference thread to cancel
    //  a running sequence of commands on a program thread.
    //

    //  Cancel all commands. This returns the controller to the neutral button
    //  state and clears the command queue.
    //  This does not wait for the commands to finish cancelling. This is an
    //  asynchronous function that merely initiates the cancellation process.
    virtual void cancel_all_commands() = 0;

    //  Same as "cancel_all_commands()", but instead of cancelling the stream,
    //  it lets it keep running. Then on the next command issued after this
    //  cancel, it will atomically replace the stream without gapping.
    //  This lets you do stuff like suddenly change joystick movement in
    //  response to inference while simultaneously holding a button without
    //  ever releasing it during the transition.
    virtual void replace_on_next_command() = 0;


public:
    //
    //  Commands
    //
    //  Commands are actions like button presses or joystick movements that are
    //  eventually sent to the console.
    //
    //  All commands are prefixed with "issue_".
    //  Commands are not thread-safe with other commands.
    //  Commands are thread-safe with the cancellation functions above.
    //
    //  Commands are asynchronous. When you call a command function on this,
    //  class it gets enqueued into a FIFO and immediately returns. It will only
    //  block if the FIFO is full.
    //
    //  If a command is called with a cancelled "cancellable" parameter, it will
    //  throw an OperationCancelledException.
    //  If a cancellation happens while you are inside a command function, it
    //  will immediately stop and throw an OperationCancelledException.
    //

    //  Wait for all unfinished commands to finish. This will also wait out
    //  hanging commands including their cooldown periods.
    //  This is not a true command function as it waits for the entire queue to
    //  empty out rather than entering itself into the queue.
    //  If a cancellation happens inside this function, it will immediately
    //  throw an OperationCancelledException.
    virtual void wait_for_all(const Cancellable* cancellable) = 0;


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
    //
    //  Superscalar Commands (the "ssf" framework)
    //

    //
    //  delay       Time to wait before moving onto the next command.
    //  hold        Time to hold the button/stick down for.
    //  cooldown    After the button has been released, prevent it from being
    //              used again for this much time.
    //
    //  For "normal" use, you should always set (delay == hold + cooldown).
    //  This is the easiest case to understand and is what the "pbf" interface
    //  exposes.
    //
    //  If the button is busy (due to still being held down or is waiting out
    //  the cooldown), the command will wait until the button is ready.
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
    //  It is important to remember that the "timeline" here is the timeline
    //  being fed to the Switch. Thus the timing parameters written in the C++
    //  code here is what you will get on the console (or as close as possible).
    //
    //  The actual calls to the methods in the class will return or block in an
    //  unspecified manner as they merely enqueue into a FIFO which is then
    //  "replayed" to the Switch in an implementation-dependent manner.
    //

    //  Tell the scheduler to wait for all pending commands to finish
    //  (including cooldowns) before executing further instructions.
    //  This is used to prevent hanging commands from overlapping with new
    //  commands issued after this barrier.
    virtual void issue_barrier(const Cancellable* cancellable) = 0;

    //  Do nothing for this much time.
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) = 0;

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

    //
    //  Press all the following buttons/joysticks simultaneously for the
    //  specified duration. No wait is added at the end. Thus you can issue
    //  these back-to-back to simulate buttons being pressed and released
    //  concurrently with other buttons being held down the whole time.
    //
    //  This command will wait until the controller is fully idle (including
    //  cooldowns) before it starts. This ensures that everything is issued
    //  simultaneously. In other words, there is an implied call to
    //  "issue_barrier()" before executing the state.
    //
    //  The sole purpose of this function is for keyboard commands.
    //  For programs, it is easier to use the individual button/joystick
    //  functions above.
    //
    //  If we need to support new Switch controller functionality
    //  (such as Joycon gyro or new stuff in Switch 2), we can simply add
    //  overloads to this and gate them behind features.
    //
    virtual void issue_full_controller_state(
        const Cancellable* cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        Milliseconds hold
    ) = 0;


public:
    //
    //  High speed Macros
    //
    //  Be mindful when calling these mashing functions on a tick imprecise
    //  controller. You can guarantee that some (most) of them will be dropped.
    //
    //  Even if you are on a tick-precise controller, it is not advised to call
    //  these if you are micromanaging with tick-level granularity. The exact
    //  timing characteristics and button selection is not specified and may be
    //  context and implementation-dependent.
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


public:
    //  Keyboard Input

    virtual void keyboard_release_all() override;
    virtual void keyboard_press(const QKeyEvent& event) override;
    virtual void keyboard_release(const QKeyEvent& event) override;


private:
    const Milliseconds m_timing_variation;

    class KeyboardManager;
    Pimpl<KeyboardManager> m_keyboard_manager;
};




//
//  The context wrapper to support asynchronous cancel.
//
class ProControllerContext final : public CancellableScope{
public:
    using ControllerType = ProController;

public:
    ProControllerContext(ProController& botbase)
        : m_controller(botbase)
    {}
    ProControllerContext(CancellableScope& parent, ProController& botbase)
        : m_controller(botbase)
    {
        attach(parent);
    }
    virtual ~ProControllerContext(){
        detach();
    }

    ProController* operator->(){
        m_lifetime_sanitizer.check_usage();
        return &m_controller;
    }

    operator ProController&() const{ return m_controller; }
    ProController& controller() const{ return m_controller; }

#if 1   //  REMOVE
    void wait_for_all_requests() const{
        auto scope = m_lifetime_sanitizer.check_scope();
        m_controller.wait_for_all(this);
    }
#endif

    //  Stop all commands in this context now.
    void cancel_now(){
        auto scope = m_lifetime_sanitizer.check_scope();
        CancellableScope::cancel(nullptr);
        m_controller.cancel_all_commands();
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
        auto scope = m_lifetime_sanitizer.check_scope();
        CancellableScope::cancel(nullptr);
        m_controller.replace_on_next_command();
    }

    virtual bool cancel(std::exception_ptr exception) noexcept override{
        auto scope = m_lifetime_sanitizer.check_scope();
        if (CancellableScope::cancel(std::move(exception))){
            return true;
        }
        try{
            m_controller.cancel_all_commands();
        }catch (...){}
        return false;
    }


private:
    ProController& m_controller;
    LifetimeSanitizer m_lifetime_sanitizer;
};





}
}
#endif
