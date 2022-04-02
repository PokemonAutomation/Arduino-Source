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

namespace PokemonAutomation{



struct CancellableScopeData{
    void operator+=(Cancellable& cancellable){
        std::lock_guard<std::mutex> lg(m_lock);
        m_children.insert(&cancellable);
    }
    void operator-=(Cancellable& cancellable){
        std::lock_guard<std::mutex> lg(m_lock);
        m_children.erase(&cancellable);
    }

    std::set<Cancellable*> m_children;

    std::mutex m_lock;
    std::condition_variable m_cv;
};



CancellableScope::CancellableScope(){}
CancellableScope::CancellableScope(CancellableScope& parent)
    : Cancellable(parent)
{}
CancellableScope::~CancellableScope(){
    detach();
}
void CancellableScope::throw_if_cancelled(){
    if (cancelled()){
        throw OperationCancelledException();
    }
}
bool CancellableScope::cancel() noexcept{
    if (Cancellable::cancel()){
        return true;
    }
    std::set<Cancellable*> children;
    {
        std::lock_guard lg(m_impl->m_lock);
        children = std::move(m_impl->m_children);
        m_impl->m_cv.notify_all();
    }
    for (Cancellable* child : children){
        child->cancel();
    }
    return false;
}
void CancellableScope::wait_for(std::chrono::milliseconds duration){
    wait_until(std::chrono::system_clock::now() + duration);
}
void CancellableScope::wait_until(std::chrono::system_clock::time_point stop){
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
    (CancellableScopeData&)m_impl += cancellable;
}
void CancellableScope::operator-=(Cancellable& cancellable){
    (CancellableScopeData&)m_impl -= cancellable;
}






}
