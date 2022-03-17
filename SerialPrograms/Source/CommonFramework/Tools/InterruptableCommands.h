/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_InterruptableCommands_H
#define PokemonAutomation_InterruptableCommands_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"


namespace PokemonAutomation{


class AsyncCommandSession{

public:
    AsyncCommandSession(ProgramEnvironment& env, BotBase& botbase);
    ~AsyncCommandSession();

    bool command_is_running();

    //  Stop the entire session. This will rethrow exceptions in the command thread.
    //  This is not thread-safe with "run()" and "stop_commands()"
    //  You must call this prior to destruction unless it's during a stack-unwind.
    void stop_session();


public:
    //  Dispath the following lambda. If something is already running, it will be
    //  stopped and replaced with this one.
    void dispatch(std::function<void(const BotBaseContext&)>&& lambda);

//    //  Stop the currently running command.
//    void stop_commands();

    //  Wait for currently running command to finish.
    void wait();


private:
    void thread_loop();


private:
    struct CommandSet{
        CommandSet(BotBase& botbase, std::function<void(const BotBaseContext&)>&& lambda);
        BotBaseContext context;
        std::function<void(const BotBaseContext&)> commands;
    };

    ProgramEnvironment& m_env;
    BotBase& m_botbase;
    std::unique_ptr<CommandSet> m_current;
//    std::unique_ptr<CommandSet> m_pending;

    std::atomic<bool> m_stopping_session;
    std::mutex m_lock;
    std::condition_variable m_cv;
//    std::thread m_thread;
//    std::exception_ptr m_exception;

    std::unique_ptr<AsyncTask> m_task;
};







}
#endif
