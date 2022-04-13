/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_InterruptableCommands_H
#define PokemonAutomation_InterruptableCommands_H

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"


namespace PokemonAutomation{

class ProgramEnvironment;



class AsyncCommandSession final : public Cancellable{
public:
    AsyncCommandSession(
        CancellableScope& scope, Logger& logger, AsyncDispatcher& dispatcher,
        BotBase& botbase
    );
    virtual ~AsyncCommandSession();

    bool command_is_running();

    //  Stop the entire session. This will rethrow exceptions in the command thread.
    //  You must call this prior to destruction unless it's during a stack-unwind.
    void stop_session_and_rethrow();


public:
    //  Dispath the following lambda. If something is already running, it will be
    //  stopped and replaced with this one.
    void dispatch(std::function<void(BotBaseContext&)>&& lambda);

//    //  Stop the currently running command.
//    void stop_commands();

//    //  Wait for currently running command to finish.
//    void wait();


private:
    virtual bool cancel(std::exception_ptr exception) noexcept override;
    void thread_loop();


private:
    struct CommandSet;

    Logger& m_logger;
    BotBase& m_botbase;
    std::unique_ptr<CommandSet> m_pending;
    std::unique_ptr<CommandSet> m_current;

    //  Finished tasks need to be moved here first and then deleted outside
    //  of "m_lock" due to a deadlock possibiliy.
    SpinLock m_finished_lock;
    std::vector<std::unique_ptr<CommandSet>> m_finished_tasks;

    std::mutex m_lock;
    std::condition_variable m_cv;
    std::unique_ptr<AsyncTask> m_thread;

    LifetimeSanitizer m_sanitizer;
};







}
#endif
