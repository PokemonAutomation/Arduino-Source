/*  Listener Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ListenerSet_H
#define PokemonAutomation_ListenerSet_H

#include <exception>
#include <list>
#include <map>
#include <atomic>
//#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/SpinLock.h"

#include <iostream>
//using std::cout;
//using std::endl;


//#define PA_DEBUG_ListenerSet


#ifdef PA_DEBUG_ListenerSet
#include "Common/Cpp/LifetimeSanitizer.h"
#endif


namespace PokemonAutomation{



template <typename ListenerType>
class ListenerSet{
public:
    ListenerSet(bool suppress_lock_prints = false);

    bool empty() const{
        return m_count.load(std::memory_order_acquire) == 0;
    }
    size_t count_unique() const{
        return m_count.load(std::memory_order_acquire);
    }

    //  Add a new listener. This is always safe as it will never fail unless it
    //  throws. Deadlocking is not possible since there's only one local lock.
    void add(ListenerType& listener);

    //  Remove a listener. This will block if the listener being removed is
    //  running a callback from this class.
    //
    //  Therefore, this will deadlock if a listener tries to remove itself from
    //  inside its own callback.
    void remove(ListenerType& listener) noexcept;

    //  Remove a listener (non-blocking). This will return false if it needs to
    //  wait. This function is always safe and will never deadlock.
    //
    //  This is intended as a safety outlet for possible self-removal. If the
    //  removal fails because it is a self-removal, it returns false so you can
    //  handle it higher up the stack.
    //
    //  Given that listeners typically remove themselves in their destructor,
    //  self-removals where a callback tries to remove itself are never safe
    //  since the class will be destructed while still deep in the call stack
    //  of its own method.
    bool try_remove(ListenerType& listener) noexcept;


public:
    //  Run the method on every listener.
    template <typename Function, class... Args>
    void run_method(Function function, Args&&... args);

    //  Run lambda on every listener.
    //  If lambda returns true, stop immediately.
    template <typename Lambda>
    void run_on_all(Lambda&& lambda);


private:
    //  Optimization. Keep an atomic version of the count. This will let us
    //  skip the lock when there are no listeners.
    std::atomic<size_t> m_count;

    const bool m_suppress_lock_prints;
    mutable SpinLock m_lock;

    //  The data structure here is (formetly) an intrusive map where the nodes
    //  form a linked-list. The map provides a fast way to add/remove listeners
    //  while the linked-list is the main method of iterating the listeners.
    //
    //  Iterating listeners to fire callbacks is completely thread-safe as they
    //  do not modify the structure of the container. OTOH, adding/removing does
    //  modify the data structure.
    //
    //  "m_lock" protects the structure of container. You cannot change the
    //  map or the linked list without holding this lock. When iterating the
    //  nodes to fire callbacks, you must hold this lock when moving from one
    //  node to the next to prevent the pointer from changing under you.
    //
    //  To prevent a listener from being removed while its callback is running,
    //  there is a lock on each node. The contract for removing is a listener is
    //  that when "remove_listener()" returns, this class holds no more
    //  references to it and thus the listener is safe to destroy.
    //
    struct Node{
        SpinLock lock;
        ListenerType& listener;

#ifdef PA_DEBUG_ListenerSet
        LifetimeSanitizer sanitizer;
#endif

        Node(ListenerSet& parent, ListenerType& p_listener)
            : listener(p_listener)
#ifdef PA_DEBUG_ListenerSet
            , sanitizer("Node")
#endif
        {}
    };
    std::list<Node> m_list;
    std::map<ListenerType*, typename std::list<Node>::iterator> m_listeners;

#ifdef PA_DEBUG_ListenerSet
    LifetimeSanitizer m_sanitizer;
#endif
};










template <typename ListenerType>
ListenerSet<ListenerType>::ListenerSet(bool suppress_lock_prints)
    : m_suppress_lock_prints(suppress_lock_prints)
{}

template <typename ListenerType>
void ListenerSet<ListenerType>::add(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    WriteSpinLock lg(m_lock, m_suppress_lock_prints ? nullptr : "ListenerSet::add()");

    auto iter = m_list.emplace(m_list.end(), *this, listener);
    try{
        auto ret = m_listeners.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(&listener),
            std::forward_as_tuple(iter)
        );
        if (!ret.second){
            m_list.erase(iter);
            return;
        }
    }catch (...){
        m_list.erase(iter);
        throw;
    }
    m_count.store(m_listeners.size(), std::memory_order_release);
}
template <typename ListenerType>
void ListenerSet<ListenerType>::remove(ListenerType& listener) noexcept{
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif

//    bool printed = false;

    while (true){
        WriteSpinLock lg(m_lock, m_suppress_lock_prints ? nullptr : "ListenerSet::remove()");
        auto iter = m_listeners.find(&listener);
        if (iter == m_listeners.end()){
            return;
        }

        typename std::list<Node>::iterator node = iter->second;

#ifdef PA_DEBUG_ListenerSet
        node->sanitizer.check_usage();
#endif

        if (!node->lock.try_acquire_write()){
#if 0
            if (!printed){
                try{
                    std::cout << "ListenerSet::remove(): Retry inner." << std::endl;
                }catch (...){}
                printed = true;
            }
#endif
            continue;
        }

//        std::cout << "node = " << &node.sanitizer << " : " << &node.prev->sanitizer << " : " << &node.next->sanitizer << std::endl;

        m_list.erase(node);
        m_listeners.erase(iter);
        m_count.store(m_listeners.size(), std::memory_order_release);
        return;
    }
}



template <typename ListenerType>
bool ListenerSet<ListenerType>::try_remove(ListenerType& listener) noexcept{
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    if (!m_lock.try_acquire_write()){
        return false;
    }
    auto iter = m_listeners.find(&listener);
    if (iter == m_listeners.end()){
        m_lock.unlock_write();
        return true;
    }

    typename std::list<Node>::iterator node = iter->second;
    if (!node->lock.try_acquire_write()){
        try{
            std::cout << "ListenerSet::try_remove(): Fail inner." << std::endl;
        }catch (...){}
        m_lock.unlock_write();
        return false;
    }

#ifdef PA_DEBUG_ListenerSet
    node->sanitizer.check_usage();
#endif

    m_list.erase(node);
    m_listeners.erase(iter);
    m_count.store(m_listeners.size(), std::memory_order_release);
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
    std::exception_ptr err;

    m_lock.acquire_read();

    for (Node& node : m_list){
        {
            ReadSpinLock lg(node.lock, m_suppress_lock_prints ? nullptr : "ListenerSet::run_method()");

#ifdef PA_DEBUG_ListenerSet
            node.sanitizer.check_usage();
#endif
            m_lock.unlock_read();
            try{
                (node.listener.*function)(std::forward<Args>(args)...);
            }catch (...){
                if (!err){
                    err = std::current_exception();
                }
            }
            m_lock.acquire_read();
        }
    }

    m_lock.unlock_read();
    if (err){
        std::rethrow_exception(err);
    }
}


template <typename ListenerType>
template <typename Lambda>
void ListenerSet<ListenerType>::run_on_all(Lambda&& lambda){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    if (empty()){
        return;
    }
    std::exception_ptr err;

    m_lock.acquire_read();

    for (Node& node : m_list){
        bool return_now = false;
        {
            ReadSpinLock lg(node.lock, m_suppress_lock_prints ? nullptr : "ListenerSet::run_on_all()");

#ifdef PA_DEBUG_ListenerSet
            node.sanitizer.check_usage();
#endif
            m_lock.unlock_read();
            try{
                return_now = lambda(node.listener);
            }catch (...){
                if (!err){
                    err = std::current_exception();
                }
            }
            m_lock.acquire_read();
        }
        if (return_now){
            break;
        }
#ifdef PA_DEBUG_ListenerSet
        node.sanitizer.check_usage();
#endif
    }

    m_lock.unlock_read();
    if (err){
        std::rethrow_exception(err);
    }
}








}
#endif
