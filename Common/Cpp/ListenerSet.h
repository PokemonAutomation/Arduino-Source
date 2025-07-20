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

    void add(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
        auto scope = m_sanitizer.check_scope();
#endif
        WriteSpinLock lg(m_lock);
//        std::lock_guard<std::mutex> lg(m_lock);
        m_listeners[&listener]++;
        m_count.store(m_listeners.size(), std::memory_order_relaxed);
    }
    void remove(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
        auto scope = m_sanitizer.check_scope();
#endif
        WriteSpinLock lg(m_lock);
//        std::lock_guard<std::mutex> lg(m_lock);
        auto iter = m_listeners.find(&listener);
        if (iter == m_listeners.end()){
            return;
        }
        if (--iter->second == 0){
            m_listeners.erase(iter);
        }
        m_count.store(m_listeners.size(), std::memory_order_relaxed);
    }

    template <typename Lambda>
    void run_lambda_unique(Lambda&& lambda){
#ifdef PA_DEBUG_ListenerSet
        auto scope = m_sanitizer.check_scope();
#endif
        if (empty()){
            return;
        }
        ReadSpinLock lg(m_lock);
//        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            lambda(*item.first);
        }
    }
    template <typename Lambda>
    void run_lambda_with_duplicates(Lambda&& lambda){
#ifdef PA_DEBUG_ListenerSet
        auto scope = m_sanitizer.check_scope();
#endif
        if (empty()){
            return;
        }
        ReadSpinLock lg(m_lock);
//        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            ListenerType& listener = *item.first;
            size_t count = item.second;
            do{
                lambda(listener);
            }while (--count);
        }
    }

    template <typename Function, class... Args>
    void run_method_unique(Function function, Args&&... args){
#ifdef PA_DEBUG_ListenerSet
        auto scope = m_sanitizer.check_scope();
#endif
        if (empty()){
            return;
        }
        ReadSpinLock lg(m_lock);
//        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            (item.first->*function)(std::forward<Args>(args)...);
        }
    }
    template <typename Function, class... Args>
    void run_method_with_duplicates(Function function, Args&&... args){
#ifdef PA_DEBUG_ListenerSet
        auto scope = m_sanitizer.check_scope();
#endif
        if (empty()){
            return;
        }
        ReadSpinLock lg(m_lock);
//        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            ListenerType& listener = *item.first;
            size_t count = item.second;
            do{
                (listener->*function)(std::forward<Args>(args)...);
            }while (--count);
        }
    }

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




}
#endif
