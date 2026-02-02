/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_InterruptableCommands_H
#define PokemonAutomation_CommonTools_InterruptableCommands_H

#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"


namespace PokemonAutomation{

class Logger;
class ProgramEnvironment;


template <typename ControllerType>
class AsyncCommandSession final : private Cancellable{
    using ControllerContextType = typename ControllerType::ContextType;

public:
    AsyncCommandSession(
        CancellableScope& scope, Logger& logger, AsyncDispatcher& dispatcher,
        ControllerType& controller
    );
    virtual ~AsyncCommandSession();

    bool command_is_running();

    //  Stop the currently running command.
    void stop_command();

    //  Dispath a controller command function `lambda` asynchronously.
    //
    //  If some commands are already running (via a previous `dispatch()` call), they
    //  will be stopped and atomically replaced with the new one.
    //
    //  This replacement maintains button states: if button A is being pressed from a
    //  previous `dispatch()` while a new `dispatch()` presses button A and button B
    //  at the same time, button A is not released during the transition.
    //  You can use it to adjust button state continuously without interruption.
    void dispatch(std::function<void(ControllerContextType&)>&& lambda);

    //  Wait for currently running command to finish.
    void wait();

    //  Stop the entire session. If an exception was thrown from the command
    //  thread, it will be rethrown here.
    //
    //  It is important that exceptions in the command thread be passed up
    //  since it is how cancellations are implemented. Therefore you MUST
    //  call this prior to all natural* destruction points.
    //
    //  *Do not call this during a stack-unwind as it's UB to throw while
    //  unwinding. In such cases, the exception from the command thread will be
    //  silently dropped in favor of the exception that's causing the current
    //  unwind.
    void stop_session_and_rethrow();

private:
    virtual bool cancel(std::exception_ptr exception) noexcept override;
    void thread_loop();


private:
    struct CommandSet;

    Logger& m_logger;
    ControllerType& m_controller;
    std::unique_ptr<CommandSet> m_current;

    Mutex m_lock;
    ConditionVariable m_cv;
    std::unique_ptr<AsyncTask> m_thread;

    LifetimeSanitizer m_sanitizer;
};







}
#endif
