/*  Async Command Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "InterruptableCommands.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



AsyncCommandSession::CommandSet::CommandSet(BotBase& botbase, std::function<void(const BotBaseContext&)>&& lambda)
    : context(botbase)
    , commands(std::move(lambda))
{}



AsyncCommandSession::AsyncCommandSession(ProgramEnvironment& env, BotBase& botbase)
    : m_env(env)
    , m_botbase(botbase)
    , m_stopping_session(false)
{
    env.register_stop_program_signal(m_lock, m_cv);
//    m_thread = std::thread(&AsyncCommandSession::thread_loop, this);
    m_task = env.dispatcher().dispatch([this]{ thread_loop(); });
}
AsyncCommandSession::~AsyncCommandSession(){
//    cout << "~AsyncCommandSession()" << endl;
    if (!m_stopping_session.load(std::memory_order_acquire) && std::uncaught_exceptions() == 0){
        m_env.log("AsyncCommandSession::stop_session() not called before normal destruction.", COLOR_RED);
    }
    if (m_task){
        m_stopping_session.store(true, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_cv.notify_all();
        }
        m_task.reset();
    }
    m_env.deregister_stop_program_signal(m_cv);
}

void AsyncCommandSession::dispatch(std::function<void(const BotBaseContext&)>&& lambda){
    std::unique_lock<std::mutex> lg(m_lock);
    m_env.check_stopping();
    if (m_stopping_session.load(std::memory_order_acquire)){
        return;
    }

    if (m_current){
        m_current->context.cancel();
        m_cv.wait(lg, [=]{
            return
                m_env.is_stopping() ||
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
                    m_env.is_stopping() ||
                    m_stopping_session.load(std::memory_order_acquire) ||
                    m_current != nullptr;
            });
            if (m_env.is_stopping() || m_stopping_session.load(std::memory_order_acquire)){
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
//                cout << "stop" << endl;
                current->context->wait_for_all_requests();
            }catch (CancelledException&){}
        }
    }
}



void AsyncCommandSession::stop_commands(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current){
        m_current->context.cancel();
    }
}
void AsyncCommandSession::wait(){
    std::unique_lock<std::mutex> lg(m_lock);
//    cout << "wait() - start" << endl;
    m_cv.wait(lg, [=]{
        return m_current == nullptr;
    });
//    cout << "wait() - done" << endl;
}
void AsyncCommandSession::stop_session(){
    m_stopping_session.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_current != nullptr){
            m_current->context.cancel();
        }
        m_cv.notify_all();
    }
    m_task->wait_and_rethrow_exceptions();
    m_env.check_stopping();
}





}

