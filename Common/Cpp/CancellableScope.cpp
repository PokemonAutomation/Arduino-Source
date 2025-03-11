/*  Cancellable Scope
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <exception>
#include <set>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Exceptions.h"
#include "Containers/Pimpl.tpp"
#include "Concurrency/SpinLock.h"
#include "CancellableScope.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct CancellableData{
    CancellableData()
        : cancelled(false)
    {}
    std::atomic<bool> cancelled;
    mutable SpinLock lock;
    std::exception_ptr exception;
};



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
bool Cancellable::cancel(std::exception_ptr exception) noexcept{
    auto scope_check = m_sanitizer.check_scope();
    CancellableData& data(*m_impl);
    WriteSpinLock lg(data.lock);
    if (exception && !data.exception){
        data.exception = std::move(exception);
    }
    if (data.cancelled.load(std::memory_order_acquire)){
        return true;
    }
    return data.cancelled.exchange(true, std::memory_order_relaxed);
}
void Cancellable::throw_if_cancelled() const{
    auto scope_check = m_sanitizer.check_scope();
    const CancellableData& data(*m_impl);
    if (!data.cancelled.load(std::memory_order_acquire)){
        return;
    }
    ReadSpinLock lg(data.lock);
    if (data.exception){
        std::rethrow_exception(data.exception);
    }else{
        throw OperationCancelledException();
    }
}
bool Cancellable::throw_if_cancelled_with_exception() const{
    auto scope_check = m_sanitizer.check_scope();
    const CancellableData& data(*m_impl);
    if (!data.cancelled.load(std::memory_order_acquire)){
        return false;
    }
    ReadSpinLock lg(data.lock);
    if (data.exception){
        std::rethrow_exception(data.exception);
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





struct CancellableScopeData{
    std::set<Cancellable*> children;

    std::mutex lock;
    std::condition_variable cv;
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
    CancellableScopeData& data(*m_impl);
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
    CancellableScopeData& data(*m_impl);
    {
        std::unique_lock<std::mutex> lg(data.lock);
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
    CancellableScopeData& data(*m_impl);
    {
        std::unique_lock<std::mutex> lg(data.lock);
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
    CancellableScopeData& data(*m_impl);
    std::lock_guard<std::mutex> lg(data.lock);
    throw_if_cancelled();
    data.children.insert(&cancellable);
}
void CancellableScope::operator-=(Cancellable& cancellable){
//    cout << "Detaching: " << &cancellable << endl;
    auto scope_check = m_sanitizer.check_scope();
    CancellableScopeData& data(*m_impl);
    std::lock_guard<std::mutex> lg(data.lock);
    data.children.erase(&cancellable);
}






}
