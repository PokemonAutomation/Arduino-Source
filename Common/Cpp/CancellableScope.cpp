/*  Cancellable Scope
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Pimpl.tpp"
#include "Exceptions.h"
#include "CancellableScope.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


CancellableScope* Cancellable::scope() const{
    m_sanitizer.check_usage();
    return m_scope;
}
bool Cancellable::cancelled() const{
    m_sanitizer.check_usage();
    return m_cancelled.load(std::memory_order_acquire);
}
bool Cancellable::cancel() noexcept{
    m_sanitizer.check_usage();
    if (cancelled() || m_cancelled.exchange(true)){
        return true;
    }
    return false;
}
void Cancellable::throw_if_cancelled(){
    m_sanitizer.check_usage();
    if (cancelled()){
        throw OperationCancelledException();
    }
}
void Cancellable::throw_if_parent_cancelled(){
    m_sanitizer.check_usage();
    if (m_scope){
        m_scope->throw_if_cancelled();
    }
}
void Cancellable::attach(CancellableScope& scope){
    m_sanitizer.check_usage();
    m_scope = &scope;
    scope += *this;
}
void Cancellable::detach() noexcept{
    m_sanitizer.check_usage();
    if (m_scope){
        *m_scope -= *this;
    }
}





struct CancellableScopeData{
    std::set<Cancellable*> m_children;

    std::mutex m_lock;
    std::condition_variable m_cv;
};



CancellableScope::CancellableScope(){}
CancellableScope::~CancellableScope(){
    detach();
}
bool CancellableScope::cancel() noexcept{
    if (Cancellable::cancel()){
        return true;
    }
    std::lock_guard lg(m_impl->m_lock);
    for (Cancellable* child : m_impl->m_children){
        child->cancel();
    }
    m_impl->m_children.clear();
    m_impl->m_cv.notify_all();
    return false;
}
void CancellableScope::wait_for(std::chrono::milliseconds duration){
    m_sanitizer.check_usage();
    wait_until(std::chrono::system_clock::now() + duration);
}
void CancellableScope::wait_until(std::chrono::system_clock::time_point stop){
    m_sanitizer.check_usage();
    throw_if_cancelled();
    {
        std::unique_lock<std::mutex> lg(m_impl->m_lock);
        m_impl->m_cv.wait_until(
            lg, stop,
            [=]{
                return std::chrono::system_clock::now() >= stop || cancelled();
            }
        );
    }
    throw_if_cancelled();
}
void CancellableScope::operator+=(Cancellable& cancellable){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_impl->m_lock);
    throw_if_cancelled();
    m_impl->m_children.insert(&cancellable);
}
void CancellableScope::operator-=(Cancellable& cancellable){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_impl->m_lock);
    m_impl->m_children.erase(&cancellable);
}






}
