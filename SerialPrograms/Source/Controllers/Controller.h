/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_Controller_H
#define PokemonAutomation_Controllers_Controller_H

#include "Common/Compiler.h"
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Time.h"
#include "Controllers/KeyboardInput/KeyboardEventHandler.h"
#include "Common/Cpp/CancellableScope.h"

class QKeyEvent;

namespace PokemonAutomation{

class RecursiveThrottler;
enum class ControllerType;
enum class ControllerPerformanceClass;
enum class ControllerClass;



inline Milliseconds round_up_to_ticksize(Milliseconds ticksize, Milliseconds duration){
    if (ticksize == Milliseconds::zero()){
        return duration;
    }
    return (duration + ticksize - Milliseconds(1)) / ticksize * ticksize;
}




class AbstractController{
public:
    static const char NAME[];

    virtual ~AbstractController() = default;

    virtual Logger& logger() = 0;
    virtual RecursiveThrottler& logging_throttler() = 0;


public:
    template <typename ControllerType>
    ControllerType* cast(){
        return dynamic_cast<ControllerType*>(this);
    }
    template <typename ControllerType>
    ControllerType& cast_with_exception(){
        ControllerType* controller = dynamic_cast<ControllerType*>(this);
        if (!controller){
            throw_bad_cast(ControllerType::NAME);
        }
        return *controller;
    }
private:
    void throw_bad_cast(const char* desired_typename);


public:
    //  Static Information

    virtual const char* name() = 0;
    virtual ControllerType controller_type() const = 0;
    virtual ControllerClass controller_class() const = 0;
    virtual ControllerPerformanceClass performance_class() const = 0;

    //  If the controller is polled at a fixed interval, this is that interval.
    //  Otherwise, returns zero.
    virtual Milliseconds ticksize() const = 0;

    //  The minimum amount of time between two state reports. This effectively
    //  limits how quickly you can change states.
    //  Controllers with non-zero ticksize will have (ticksize == cooldown).
    virtual Milliseconds cooldown() const = 0;

    //  Some controllers are imprecise. This returns the variation.
    //  Zero means "tick precise".
    virtual Milliseconds timing_variation() const = 0;

    //  If the controller can atomically press/release multiple buttons
    //  return true. This means that if the program presses A and B
    //  simultaneously, the console will never see an intermediate state where
    //  either A or B is pressed by itself before the other is pressed as well.
    virtual bool atomic_multibutton() const = 0;


public:
    //  Status

    virtual bool is_ready() const = 0;


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

    //  Wait for all unfinished commands to finish. This will also wait out
    //  hanging commands including their cooldown periods.
    //  This is not a true command function as it waits for the entire queue to
    //  empty out rather than entering itself into the queue.
    //  If a cancellation happens inside this function, it will immediately
    //  throw an OperationCancelledException.
    virtual void wait_for_all(const Cancellable* cancellable) = 0;

    //  Tell the scheduler to wait for all pending commands to finish
    //  (including cooldowns) before executing further instructions.
    //  This is used to prevent hanging commands from overlapping with new
    //  commands issued after this barrier.
    virtual void issue_barrier(const Cancellable* cancellable) = 0;

    //  Do nothing for this much time.
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) = 0;


public:
    //  Keyboard Controls

    virtual void keyboard_release_all(){}
    virtual void keyboard_press(const QKeyEvent& event){}
    virtual void keyboard_release(const QKeyEvent& event){}

    virtual void add_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){};
    virtual void remove_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){};
};


inline const char AbstractController::NAME[] = "Controller";



//
//  The context wrapper to support asynchronous cancel.
//
template <typename Type>
class ControllerContext final : public CancellableScope{
public:
    using ControllerType = Type;


public:
    virtual ~ControllerContext(){
        detach();
    }
    ControllerContext(ControllerType& controller)
        : m_controller(controller)
    {}
    ControllerContext(CancellableScope& parent, ControllerType& controller)
        : m_controller(controller)
    {
        attach(parent);
    }
    template <typename T>
    ControllerContext(ControllerContext<T>& context)
        : m_controller(context.controller().template cast_with_exception<Type>())
    {
        attach(context);
    }


public:
    ControllerType* operator->(){
        m_lifetime_sanitizer.check_usage();
        return &m_controller;
    }

    operator ControllerType&() const{ return m_controller; }
    ControllerType& controller() const{ return m_controller; }


public:
    void wait_for_all_requests() const{
        auto scope = m_lifetime_sanitizer.check_scope();
        m_controller.wait_for_all(this);
    }

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
    ControllerType& m_controller;
    LifetimeSanitizer m_lifetime_sanitizer;
};


using AbstractControllerContext = ControllerContext<AbstractController>;

using AbstractControllerContext = ControllerContext<AbstractController>;




}
#endif
