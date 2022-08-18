/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_InterruptableCommands_H
#define PokemonAutomation_InterruptableCommands_H

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"


namespace PokemonAutomation{

class ProgramEnvironment;



class AsyncCommandSession final : private Cancellable{
public:
    AsyncCommandSession(
        CancellableScope& scope, Logger& logger, AsyncDispatcher& dispatcher,
        BotBase& botbase
    );
    virtual ~AsyncCommandSession();

    bool command_is_running();

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


public:
    //  Dispath the following lambda. If something is already running, it will be
    //  stopped and replaced with this one.
    void dispatch(std::function<void(BotBaseContext&)>&& lambda);

//    //  Stop the currently running command.
//    void stop_commands();

    //  Wait for currently running command to finish.
    void wait();


private:
    virtual bool cancel(std::exception_ptr exception) noexcept override;
    void thread_loop();


private:
    struct CommandSet;

    Logger& m_logger;
    BotBase& m_botbase;
    std::unique_ptr<CommandSet> m_current;

    std::mutex m_lock;
    std::condition_variable m_cv;
    std::unique_ptr<AsyncTask> m_thread;

    LifetimeSanitizer m_sanitizer;
};







}
#endif
