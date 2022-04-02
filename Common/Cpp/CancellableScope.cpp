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



class CancellableScopeImpl{
public:
    CancellableScopeImpl()
        : m_stopped(false)
    {
//        cout << "CancellableScope(): " << this << endl;
    }

    bool cancelled() const{
        return m_stopped.load(std::memory_order_acquire);
    }
    void check_cancelled(){
        if (cancelled()){
            throw OperationCancelledException();
        }
    }
    void cancel() noexcept{
//        cout << "cancel(): " << this << endl;
        if (cancelled()){
            return;
        }

        std::set<Cancellable*> children;
        {
            std::lock_guard lg(m_lock);
            if (m_stopped.load(std::memory_order_acquire)){
                return;
            }
            m_stopped.store(true, std::memory_order_release);
            children = std::move(m_children);
            m_cv.notify_all();
        }
        for (Cancellable* child : children){
            child->cancel();
        }
    }
    void wait_for(std::chrono::milliseconds duration){
        wait_until(std::chrono::system_clock::now() + duration);
    }
    void wait_until(std::chrono::system_clock::time_point stop){
        check_cancelled();
        {
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.wait_until(
                lg, stop,
                [=]{
                    return std::chrono::system_clock::now() >= stop || cancelled();
                }
            );
        }
        check_cancelled();
    }


private:
    friend class CancellableScope;
    void operator+=(Cancellable& cancellable){
        std::lock_guard<std::mutex> lg(m_lock);
        m_children.insert(&cancellable);
    }
    void operator-=(Cancellable& cancellable){
        std::lock_guard<std::mutex> lg(m_lock);
        m_children.erase(&cancellable);
    }


private:
    std::atomic<bool> m_stopped;
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
bool CancellableScope::cancelled() const{
    return m_impl->cancelled();
}
void CancellableScope::check_cancelled(){
    m_impl->check_cancelled();
}
void CancellableScope::cancel() noexcept{
    m_impl->cancel();
}
void CancellableScope::wait_for(std::chrono::milliseconds duration){
    m_impl->wait_for(duration);
}
void CancellableScope::wait_until(std::chrono::system_clock::time_point stop){
    m_impl->wait_until(stop);
}
void CancellableScope::operator+=(Cancellable& cancellable){
    (CancellableScopeImpl&)m_impl += cancellable;
}
void CancellableScope::operator-=(Cancellable& cancellable){
    (CancellableScopeImpl&)m_impl -= cancellable;
}



}
