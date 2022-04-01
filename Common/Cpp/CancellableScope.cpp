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
        , m_parent(nullptr)
    {
//        cout << "CancellableScope(): " << this << endl;
    }
    CancellableScopeImpl(CancellableScopeImpl& parent)
        : m_stopped(false)
        , m_parent(&parent)
    {
//        cout << "CancellableScope(parent): " << this << endl;
        parent.add_child(*this);
    }
    ~CancellableScopeImpl(){
//        cout << "~CancellableScope(): " << this << endl;
        cancel();
        {
            std::unique_lock lg(m_lock);
            m_cv.wait(lg, [=]{ return m_children.empty(); });
        }
        if (m_parent){
            m_parent->remove_child(*this);
        }
    }

    bool stopped() const{
        return m_stopped.load(std::memory_order_acquire);
    }
    void check_stopped(){
        if (stopped()){
            throw OperationCancelledException();
        }
    }
    void cancel(){
//        cout << "cancel(): " << this << endl;
        if (stopped()){
            return;
        }
        m_stopped.store(true, std::memory_order_release);
        std::lock_guard lg(m_lock);
        for (CancellableScopeImpl* child : m_children){
            child->cancel();
        }
        m_cv.notify_all();
    }
    void wait_for(std::chrono::milliseconds duration){
        wait_until(std::chrono::system_clock::now() + duration);
    }
    void wait_until(std::chrono::system_clock::time_point stop){
        check_stopped();
        {
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.wait_until(
                lg, stop,
                [=]{
                    return std::chrono::system_clock::now() >= stop || stopped();
                }
            );
        }
        check_stopped();
    }


private:
    void add_child(CancellableScopeImpl& child){
        std::lock_guard<std::mutex> lg(m_lock);
        if (stopped()){
            throw OperationCancelledException();
        }
        m_children.insert(&child);
    }
    void remove_child(CancellableScopeImpl& child){
        std::lock_guard<std::mutex> lg(m_lock);
        m_children.erase(&child);
        m_cv.notify_all();
    }


private:
    std::atomic<bool> m_stopped;
    CancellableScopeImpl* m_parent;
    std::set<CancellableScopeImpl*> m_children;

    std::mutex m_lock;
    std::condition_variable m_cv;
};



CancellableScope::CancellableScope(){}
CancellableScope::CancellableScope(CancellableScope& parent)
    : m_impl(parent.m_impl)
{}
CancellableScope::~CancellableScope(){}

bool CancellableScope::stopped() const{
    return m_impl->stopped();
}
void CancellableScope::check_stopped(){
    m_impl->check_stopped();
}
void CancellableScope::cancel(){
    m_impl->cancel();
}
void CancellableScope::wait_for(std::chrono::milliseconds duration){
    m_impl->wait_for(duration);
}
void CancellableScope::wait_until(std::chrono::system_clock::time_point stop){
    m_impl->wait_until(stop);
}



}
