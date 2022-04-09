/*  Async Command Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "InterruptableCommands.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



struct AsyncCommandSession::CommandSet{
    CommandSet(
        CancellableScope& parent, BotBase& botbase,
        std::function<void(BotBaseContext&)>&& lambda
    );
    CancellableHolder<CancellableScope> scope;
    BotBaseContext context;
    std::function<void(BotBaseContext&)> commands;
};



AsyncCommandSession::CommandSet::CommandSet(
    CancellableScope& parent, BotBase& botbase,
    std::function<void(BotBaseContext&)>&& lambda
)
    : scope(parent)
    , context(scope, botbase)
    , commands(std::move(lambda))
{}



AsyncCommandSession::AsyncCommandSession(
    CancellableScope& scope, Logger& logger, AsyncDispatcher& dispatcher,
    BotBase& botbase
)
    : m_logger(logger)
    , m_botbase(botbase)
    , m_thread(dispatcher.dispatch([this]{ thread_loop(); }))
{
    attach(scope);
}
AsyncCommandSession::~AsyncCommandSession(){
//    cout << "~AsyncCommandSession()" << endl;
    if (!cancelled() && std::uncaught_exceptions() == 0){
        m_logger.log("AsyncCommandSession::stop_session() not called before normal destruction.", COLOR_RED);
    }
    detach();
    AsyncCommandSession::cancel(nullptr);

    //  Join the thread.
    m_thread.reset();
}

bool AsyncCommandSession::command_is_running(){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    return m_current != nullptr;
}

void AsyncCommandSession::dispatch(std::function<void(BotBaseContext&)>&& lambda){
    m_sanitizer.check_usage();
    std::unique_lock<std::mutex> lg(m_lock);
    if (cancelled()){
        return;
    }

    //  Set the new task.
    m_pending.reset(new CommandSet(
        *this->scope(),
        m_botbase, std::move(lambda)
    ));

    if (m_current){
        //  Already a task running. Cancel it.
        m_current->context.cancel_lazy();
    }else{
        //  Otherwise, wake up the thread.
        m_cv.notify_all();
    }
}


bool AsyncCommandSession::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(exception)){
        return true;
    }
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current != nullptr){
        m_current->context.cancel(std::move(exception));
    }
    m_cv.notify_all();
    return false;
}
void AsyncCommandSession::thread_loop(){
//    CommandSet* current = nullptr;
    while (true){
        CommandSet* current = nullptr;
        {
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.wait(lg, [=]{
                return cancelled() || m_pending != nullptr;
            });
            if (cancelled()){
                break;
            }
            m_current = std::move(m_pending);
            current = m_current.get();
        }
        try{
//            cout << "start" << endl;
            current->commands(current->context);
            current->context.wait_for_all_requests();
//            cout << "stop" << endl;
        }catch (OperationCancelledException&){}

        //  Transfer finished task to the finished queue under both locks.
        {
            std::unique_lock<std::mutex> lg(m_lock);
            SpinLockGuard lg1(m_finished_lock);
            m_finished_tasks.emplace_back(std::move(m_current));
        }

        //  Now it's safe to remove the finished task under just the finished lock.
        {
            SpinLockGuard lg(m_finished_lock);
            m_finished_tasks.clear();
        }
    }

    SpinLockGuard lg(m_finished_lock);
    m_finished_tasks.clear();

    m_sanitizer.check_usage();
}



#if 0
void AsyncCommandSession::stop_commands(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current){
        m_current->context.cancel_now();
    }
}
#endif
#if 0
void AsyncCommandSession::wait(){
    std::unique_lock<std::mutex> lg(m_lock);
//    cout << "wait() - start" << endl;
    m_cv.wait(lg, [=]{
        return m_current == nullptr;
    });
//    cout << "wait() - done" << endl;
}
#endif
void AsyncCommandSession::stop_session_and_rethrow(){
    cancel(nullptr);
    m_thread->wait_and_rethrow_exceptions();
    throw_if_cancelled_with_exception();
}





}

