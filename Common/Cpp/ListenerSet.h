/*  Listener Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ListenerSet_H
#define PokemonAutomation_ListenerSet_H

#include <map>
#include <atomic>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/LifetimeSanitizer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

//#define PA_DEBUG_ListenerSet


namespace PokemonAutomation{



template <typename ListenerType>
class ListenerSet{
public:
    bool empty() const{
        return m_count.load(std::memory_order_relaxed) == 0;
    }
    size_t count_unique() const{
        return m_count.load(std::memory_order_relaxed);
    }

    void add(ListenerType& listener);
    void remove(ListenerType& listener);

    bool try_add(ListenerType& listener);
    bool try_remove(ListenerType& listener);

    template <typename Function, class... Args>
    void run_method(Function function, Args&&... args);

private:
    //  Optimization. Keep an atomic version of the count. This will let us
    //  skip the lock when there are no listeners.
    std::atomic<size_t> m_count;

    mutable SpinLock m_lock;
//    mutable std::mutex m_lock;
    std::map<ListenerType*, size_t> m_listeners;

#ifdef PA_DEBUG_ListenerSet
    LifetimeSanitizer m_sanitizer;
#endif
};











template <typename ListenerType>
void ListenerSet<ListenerType>::add(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    WriteSpinLock lg(m_lock);
    m_listeners[&listener]++;
    m_count.store(m_listeners.size(), std::memory_order_relaxed);
}
template <typename ListenerType>
void ListenerSet<ListenerType>::remove(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    WriteSpinLock lg(m_lock);
    auto iter = m_listeners.find(&listener);
    if (iter == m_listeners.end()){
        return;
    }
    if (--iter->second == 0){
        m_listeners.erase(iter);
    }
    m_count.store(m_listeners.size(), std::memory_order_relaxed);
}



template <typename ListenerType>
bool ListenerSet<ListenerType>::try_add(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    if (!m_lock.try_acquire_write()){
        return false;
    }
    m_listeners[&listener]++;
    m_count.store(m_listeners.size(), std::memory_order_relaxed);
    m_lock.unlock_write();
    return true;
}
template <typename ListenerType>
bool ListenerSet<ListenerType>::try_remove(ListenerType& listener){
    if (!m_lock.try_acquire_write()){
        return false;
    }
    auto iter = m_listeners.find(&listener);
    if (iter == m_listeners.end()){
        m_lock.unlock_write();
        return true;
    }
    if (--iter->second == 0){
        m_listeners.erase(iter);
    }
    m_count.store(m_listeners.size(), std::memory_order_relaxed);
    m_lock.unlock_write();
    return true;
}



template <typename ListenerType>
template <typename Function, class... Args>
void ListenerSet<ListenerType>::run_method(Function function, Args&&... args){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    if (empty()){
        return;
    }
    ReadSpinLock lg(m_lock);
    for (auto& item : m_listeners){
        (item.first->*function)(std::forward<Args>(args)...);
    }
}










}
#endif
