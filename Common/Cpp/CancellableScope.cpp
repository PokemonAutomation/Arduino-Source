/*  Cancellable Scope
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <exception>
#include <set>
#include <atomic>
#include "Exceptions.h"
#include "ListenerSet.h"
#include "Containers/Pimpl.tpp"
#include "Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "CancellableScope.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct Cancellable::Data{
    Data()
        : cancelled(false)
    {}
    std::atomic<bool> cancelled;
    mutable SpinLock lock;
    std::exception_ptr cancel_reason;
    ListenerSet<Cancellable::CancelListener> m_listeners;
};


void Cancellable::add_cancel_listener(CancelListener& listener){
    m_impl->m_listeners.add(listener);
}
void Cancellable::remove_cancel_listener(CancelListener& listener) noexcept{
    m_impl->m_listeners.remove(listener);
}


Cancellable::Cancellable()
    : m_impl(CONSTRUCT_TOKEN)
{
//    cout << "Constructing: " << this << endl;
}
Cancellable::~Cancellable(){
    detach();
//    cout << "Deleting: " << this << endl;
}
CancellableScope* Cancellable::scope() const{
    auto scope_check = m_sanitizer.check_scope();
    return m_scope;
}
bool Cancellable::cancelled() const noexcept{
    auto scope_check = m_sanitizer.check_scope();
    return m_impl->cancelled.load(std::memory_order_acquire);
}
std::exception_ptr Cancellable::cancel_reason() const{
    auto scope_check = m_sanitizer.check_scope();
    ReadSpinLock lg(m_impl->lock);
    return m_impl->cancel_reason;
}
bool Cancellable::cancel(std::exception_ptr reason) noexcept{
    auto scope_check = m_sanitizer.check_scope();
    Data& data(*m_impl);
    WriteSpinLock lg(data.lock);
    if (reason && !data.cancel_reason){
        data.cancel_reason = std::move(reason);
    }
    if (data.cancelled.load(std::memory_order_relaxed)){
        return true;
    }
    data.cancelled.store(true, std::memory_order_release);
//    if (data.cancelled.exchange(true, std::memory_order_relaxed)){
//        return true;
//    }
    if (!m_impl->m_listeners.empty()){
        m_impl->m_listeners.run_method(&CancelListener::on_cancellable_cancel, data.cancel_reason);
    }
    return false;
}
void Cancellable::throw_if_cancelled() const{
    auto scope_check = m_sanitizer.check_scope();
    const Data& data(*m_impl);
    if (!data.cancelled.load(std::memory_order_acquire)){
        return;
    }
    ReadSpinLock lg(data.lock);
    if (data.cancel_reason){
        std::rethrow_exception(data.cancel_reason);
    }else{
        throw OperationCancelledException();
    }
}
bool Cancellable::throw_if_cancelled_with_exception() const{
    auto scope_check = m_sanitizer.check_scope();
    const Data& data(*m_impl);
    if (!data.cancelled.load(std::memory_order_acquire)){
        return false;
    }
    ReadSpinLock lg(data.lock);
    if (data.cancel_reason){
        std::rethrow_exception(data.cancel_reason);
    }
    return true;
}
void Cancellable::attach(CancellableScope& scope){
    auto scope_check = m_sanitizer.check_scope();
    m_scope = &scope;
    scope += *this;
}
void Cancellable::detach() noexcept{
    auto scope_check = m_sanitizer.check_scope();
    if (m_scope){
        *m_scope -= *this;
    }
}





struct CancellableScope::Data{
    std::set<Cancellable*> children;

    Mutex lock;
    ConditionVariable cv;
};



CancellableScope::CancellableScope()
    : m_impl(CONSTRUCT_TOKEN)
{}
CancellableScope::~CancellableScope(){
    detach();
}
bool CancellableScope::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(exception)){
        return true;
    }
    Data& data(*m_impl);
    std::lock_guard lg(data.lock);
    for (Cancellable* child : data.children){
//        cout << "Canceling: " << child << endl;
//        cout << "Canceling: " << child->name() << endl;
        auto scope_check = child->m_sanitizer.check_scope();
        child->cancel(exception);
    }
//    cout << "Done Canceling" << endl;
    data.children.clear();
    data.cv.notify_all();
    return false;
}
void CancellableScope::wait_for(std::chrono::milliseconds duration){
    auto scope_check = m_sanitizer.check_scope();
    wait_until(current_time() + duration);
}
void CancellableScope::wait_until(WallClock stop){
    auto scope_check = m_sanitizer.check_scope();
    throw_if_cancelled();
    Data& data(*m_impl);
    {
        std::unique_lock<Mutex> lg(data.lock);
        data.cv.wait_until(
            lg, stop,
            [this, stop]{
                return current_time() >= stop || cancelled();
            }
        );
    }
    throw_if_cancelled();
}
void CancellableScope::wait_until_cancel(){
    auto scope_check = m_sanitizer.check_scope();
    throw_if_cancelled();
    Data& data(*m_impl);
    {
        std::unique_lock<Mutex> lg(data.lock);
        data.cv.wait(
            lg,
            [this]{
                return cancelled();
            }
        );
    }
    throw_if_cancelled();
}
void CancellableScope::operator+=(Cancellable& cancellable){
//    cout << "Attaching: " << &cancellable << endl;
    auto scope_check = m_sanitizer.check_scope();
    Data& data(*m_impl);
    std::lock_guard<Mutex> lg(data.lock);
    throw_if_cancelled();
    data.children.insert(&cancellable);
}
void CancellableScope::operator-=(Cancellable& cancellable){
//    cout << "Detaching: " << &cancellable << endl;
    auto scope_check = m_sanitizer.check_scope();
    Data& data(*m_impl);
    std::lock_guard<Mutex> lg(data.lock);
    data.children.erase(&cancellable);
}






}
