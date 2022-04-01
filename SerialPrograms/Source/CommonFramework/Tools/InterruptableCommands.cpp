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



AsyncCommandSession::CommandSet::CommandSet(BotBase& botbase, std::function<void(const BotBaseContext&)>&& lambda)
    : context(botbase)
    , commands(std::move(lambda))
{}



AsyncCommandSession::AsyncCommandSession(
    CancellableScope& scope, Logger& logger, AsyncDispatcher& dispatcher,
    BotBase& botbase
)
    : Cancellable(scope)
    , m_logger(logger)
    , m_botbase(botbase)
    , m_stopping_session(false)
{
    m_task = dispatcher.dispatch([this]{ thread_loop(); });
}
AsyncCommandSession::~AsyncCommandSession(){
//    cout << "~AsyncCommandSession()" << endl;
    if (!m_stopping_session.load(std::memory_order_acquire) && std::uncaught_exceptions() == 0){
        m_logger.log("AsyncCommandSession::stop_session() not called before normal destruction.", COLOR_RED);
    }
    detach();
    if (m_task){
        m_stopping_session.store(true, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lg(m_lock);
            if (m_current){
                try{
                    m_current->context.cancel_now();
                }catch (InvalidConnectionStateException&){
                }catch (OperationCancelledException&){
                }catch (...){
                    m_logger.log("AsyncCommandSession::~AsyncCommandSession() - Uncaught Exception", COLOR_RED);
                }
            }
            m_cv.notify_all();
        }
        m_task.reset();
    }
}

bool AsyncCommandSession::command_is_running(){
    std::lock_guard<std::mutex> lg(m_lock);
    return m_current != nullptr;
}

void AsyncCommandSession::dispatch(std::function<void(const BotBaseContext&)>&& lambda){
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_stopping_session.load(std::memory_order_acquire)){
        return;
    }

    if (m_current){
        m_current->context.cancel_lazy();
        m_cv.wait(lg, [=]{
            return
                m_stopping_session.load(std::memory_order_acquire) ||
                m_current == nullptr;
        });
    }

//    cout << "dispatching" << endl;

    m_current.reset(new CommandSet(
        m_botbase, std::move(lambda)
    ));

    m_cv.notify_all();
}


void AsyncCommandSession::cancel(){

}
void AsyncCommandSession::thread_loop(){
    CommandSet* current = nullptr;
    while (true){
        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (current != nullptr){
                m_current = nullptr;
                m_cv.notify_all();
            }
            m_cv.wait(lg, [=]{
                return
                    m_stopping_session.load(std::memory_order_acquire) ||
                    m_current != nullptr;
            });
            if (m_stopping_session.load(std::memory_order_acquire)){
//                cout << "thread_loop() - end" << endl;
                m_current.reset();
                return;
            }
            current = m_current.get();
        }

        if (current != nullptr){
            try{
//                cout << "start" << endl;
                current->commands(current->context);
                current->context.wait_for_all_requests();
//                cout << "stop" << endl;
            }catch (OperationCancelledException&){
            }
        }
    }
}



#if 0
void AsyncCommandSession::stop_commands(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current){
        m_current->context.cancel_now();
    }
}
#endif
void AsyncCommandSession::wait(){
    std::unique_lock<std::mutex> lg(m_lock);
//    cout << "wait() - start" << endl;
    m_cv.wait(lg, [=]{
        return m_current == nullptr;
    });
//    cout << "wait() - done" << endl;
}
void AsyncCommandSession::stop_session(){
    bool expected = false;
    if (!m_stopping_session.compare_exchange_strong(expected, true)){
        {
            std::lock_guard<std::mutex> lg(m_lock);
            if (m_current != nullptr){
                m_current->context.cancel_now();
            }
            m_cv.notify_all();
        }
        m_task->wait_and_rethrow_exceptions();
    }
    check_parent_cancelled();
}





}

