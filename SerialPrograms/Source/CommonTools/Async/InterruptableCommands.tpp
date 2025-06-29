/*  Async Command Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "InterruptableCommands.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



template <typename ControllerType>
struct AsyncCommandSession<ControllerType>::CommandSet{
    CommandSet(
        CancellableScope& parent, ControllerType& controller,
        std::function<void(ControllerContextType&)>&& lambda
    );
    CancellableHolder<CancellableScope> scope;
    ControllerContextType context;
    std::function<void(ControllerContextType&)> commands;
};



template <typename ControllerType>
AsyncCommandSession<ControllerType>::CommandSet::CommandSet(
    CancellableScope& parent, ControllerType& controller,
    std::function<void(ControllerContextType&)>&& lambda
)
    : scope(parent)
    , context(scope, controller)
    , commands(std::move(lambda))
{}



template <typename ControllerType>
AsyncCommandSession<ControllerType>::AsyncCommandSession(
    CancellableScope& scope, Logger& logger, AsyncDispatcher& dispatcher,
    ControllerType& controller
)
    : m_logger(logger)
    , m_controller(controller)
{
    //  Attach first. If scope is already cancelled, we exit here with nothing
    //  to clean up.
    attach(scope);

    //  Now start the thread. Destructor is guaranteed to run if this succeeds.
    m_thread = dispatcher.dispatch([this]{ thread_loop(); });
}
template <typename ControllerType>
AsyncCommandSession<ControllerType>::~AsyncCommandSession(){
//    cout << "~AsyncCommandSession()" << endl;
    if (!cancelled() && std::uncaught_exceptions() == 0){
        m_logger.log("AsyncCommandSession::stop_session() not called before normal destruction.", COLOR_RED);
    }
    detach();
    AsyncCommandSession::cancel(nullptr);

    //  Join the thread.
    m_thread.reset();
}

template <typename ControllerType>
bool AsyncCommandSession<ControllerType>::command_is_running(){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    return m_current != nullptr;
}

template <typename ControllerType>
void AsyncCommandSession<ControllerType>::stop_command(){
    auto scope_check = m_sanitizer.check_scope();

    std::unique_lock<std::mutex> lg(m_lock);
    if (cancelled()){
        return;
    }

    if (m_current){
        //  Already a task running. Cancel it.
        m_current->context.cancel_now();
        m_cv.wait(lg, [this]{
            return cancelled() || m_current == nullptr;
        });
    }
}
template <typename ControllerType>
void AsyncCommandSession<ControllerType>::dispatch(std::function<void(ControllerContextType&)>&& lambda){
    auto scope_check = m_sanitizer.check_scope();

    //  Construct the CommandSet outside the lock.
    //  If a cancellation comes from above while we are holding the lock,
    //  it will deadlock.
    std::unique_ptr<CommandSet> pending(new CommandSet(
        *this->scope(),
        m_controller, std::move(lambda)
    ));

    std::unique_lock<std::mutex> lg(m_lock);
    if (cancelled()){
        return;
    }

    if (m_current){
        //  Already a task running. Cancel it.
        m_current->context.cancel_lazy();
        m_cv.wait(lg, [this]{
            return cancelled() || m_current == nullptr;
        });
        if (cancelled()){
            return;
        }
    }

    m_current = std::move(pending);
    m_cv.notify_all();
}


template <typename ControllerType>
bool AsyncCommandSession<ControllerType>::cancel(std::exception_ptr exception) noexcept{
//    cout << "AsyncCommandSession::cancel()" << endl;
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
template <typename ControllerType>
void AsyncCommandSession<ControllerType>::thread_loop(){
    while (true){
        CommandSet* current;
        {
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.wait(lg, [this]{
                return cancelled() || m_current != nullptr;
            });
            if (cancelled()){
                break;
            }
            current = m_current.get();
        }
        try{
            current->commands(current->context);
            current->context.wait_for_all_requests();
        }catch (OperationCancelledException&){}

        //  Destroy the CommandSet outside the lock.
        //  If a cancellation comes from above while we are holding the lock,
        //  it will deadlock.
        std::unique_ptr<CommandSet> done;
        {
            std::lock_guard<std::mutex> lg(m_lock);
            done = std::move(m_current);
            m_cv.notify_all();
        }
    }

    auto scope_check = m_sanitizer.check_scope();
}



#if 0
template <typename ControllerType>
void AsyncCommandSession::stop_commands(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current){
        m_current->context.cancel_now();
    }
}
#endif
template <typename ControllerType>
void AsyncCommandSession<ControllerType>::wait(){
    std::unique_lock<std::mutex> lg(m_lock);
//    cout << "wait() - start" << endl;
    m_cv.wait(lg, [this]{
        return m_current == nullptr;
    });
//    cout << "wait() - done" << endl;
}
template <typename ControllerType>
void AsyncCommandSession<ControllerType>::stop_session_and_rethrow(){
    cancel(nullptr);
    m_thread->wait_and_rethrow_exceptions();
    throw_if_cancelled_with_exception();
}




}

