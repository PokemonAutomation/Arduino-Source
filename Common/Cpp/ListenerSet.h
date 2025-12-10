/*  Listener Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ListenerSet_H
#define PokemonAutomation_ListenerSet_H

#include <exception>
#include <map>
#include <atomic>
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
    bool empty() const{
        return m_count.load(std::memory_order_acquire) == 0;
    }
    size_t count_unique() const{
        return m_count.load(std::memory_order_acquire);
    }

    //  Add a new listener. This will never fail unless it throws.
    //  Deadlocking is not possible since there's only one local lock.
    void add(ListenerType& listener);

    //  Remove a listener. This will deadlock if a listener tries to remove
    //  from inside a callback.
    void remove(ListenerType& listener);

    //  Same as above, but will return false if it needs to wait.
    //  This can never deadlock.
    bool try_remove(ListenerType& listener);

    template <typename Function, class... Args>
    void run_method(Function function, Args&&... args);


private:
    //  Optimization. Keep an atomic version of the count. This will let us
    //  skip the lock when there are no listeners.
    std::atomic<size_t> m_count;

    mutable SpinLock m_lock;

    struct Node{
        SpinLock lock;
        ListenerType& listener;
        Node* next = nullptr;
        Node** prevs_next = nullptr;

#ifdef PA_DEBUG_ListenerSet
        LifetimeSanitizer sanitizer;
#endif

        Node(ListenerSet& parent, ListenerType& p_listener)
            : listener(p_listener)
            , prevs_next(&parent.m_list)
#ifdef PA_DEBUG_ListenerSet
            , sanitizer("Node")
#endif
        {}
    };
    Node* m_list = nullptr;
    std::map<ListenerType*, Node> m_listeners;

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
    auto ret = m_listeners.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(&listener),
        std::forward_as_tuple(*this, listener)
        );
    if (!ret.second){
        return;
    }
    Node& node = ret.first->second;
#ifdef PA_DEBUG_ListenerSet
    node.sanitizer.check_usage();
#endif
    if (m_list != nullptr){
        m_list->prevs_next = &node.next;
    }
    node.next = m_list;
    m_list = &node;
    m_count.store(m_listeners.size(), std::memory_order_relaxed);
}
template <typename ListenerType>
void ListenerSet<ListenerType>::remove(ListenerType& listener){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    while (true){
        WriteSpinLock lg(m_lock);
        auto iter = m_listeners.find(&listener);
        if (iter == m_listeners.end()){
            return;
        }

        Node& node = iter->second;

#ifdef PA_DEBUG_ListenerSet
        node.sanitizer.check_usage();
#endif

        if (!node.lock.try_acquire_write()){
            std::cout << "ListenerSet::remove(): Retry inner." << std::endl;
            continue;
        }

//        std::cout << "node = " << &node.sanitizer << " : " << &node.prev->sanitizer << " : " << &node.next->sanitizer << std::endl;

        *node.prevs_next = node.next;
        if (node.next){
#ifdef PA_DEBUG_ListenerSet
            node.next->sanitizer.check_usage();
#endif
            node.next->prevs_next = node.prevs_next;
        }

#ifdef PA_DEBUG_ListenerSet
        node.sanitizer.check_usage();
#endif

        m_listeners.erase(iter);
        m_count.store(m_listeners.size(), std::memory_order_relaxed);
        return;
    }
}



template <typename ListenerType>
bool ListenerSet<ListenerType>::try_remove(ListenerType& listener){
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

    Node& node = iter->second;
    if (!node.lock.try_acquire_write()){
        std::cout << "ListenerSet::try_remove(): Fail inner." << std::endl;
        return false;
    }

#ifdef PA_DEBUG_ListenerSet
    node.sanitizer.check_usage();
#endif

    *node.prevs_next = node.next;
    if (node.next){
#ifdef PA_DEBUG_ListenerSet
        node.next->sanitizer.check_usage();
#endif
        node.next->prevs_next = node.prevs_next;
    }

#ifdef PA_DEBUG_ListenerSet
    node.sanitizer.check_usage();
#endif

    m_listeners.erase(iter);
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
    std::exception_ptr err;

    m_lock.acquire_read();

    Node* node = m_list;
    while (node){
        {
            ReadSpinLock lg(node->lock);

#ifdef PA_DEBUG_ListenerSet
            node->sanitizer.check_usage();
#endif
            m_lock.unlock_read();
            try{
                (node->listener.*function)(std::forward<Args>(args)...);
            }catch (...){
                if (!err){
                    err = std::current_exception();
                }
            }
            m_lock.acquire_read();
        }
#ifdef PA_DEBUG_ListenerSet
        node->sanitizer.check_usage();
#endif
        node = node->next;
    }

    m_lock.unlock_read();
    if (err){
        std::rethrow_exception(err);
    }
}










}
#endif
