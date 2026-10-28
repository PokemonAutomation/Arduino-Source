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
#include <iostream>
#include "Common/Cpp/Concurrency/SpinLock.h"


//#define PA_DEBUG_ListenerSet


#ifdef PA_DEBUG_ListenerSet
#include "Common/Cpp/LifetimeSanitizer.h"
#endif


namespace PokemonAutomation{

class UnitTestDatabase;


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

    //
    //  Add a new listener. This is always safe as it will never fail unless it
    //  throws. Deadlocking is not possible since there's only one local lock.
    //

    //  Add a new listener to the front of the call order, so it fires before
    //  all existing listeners.
    void push_front(ListenerType& listener);

    //  Add a new listener to the back of the call order, so it fires after all
    //  existing listeners.
    void push_back(ListenerType& listener);

    //  Same as "push_back()".
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
    //  The data structure here is an intrusive map where the nodes form a
    //  linked-list. The map provides a fast way to add/remove listeners while
    //  the linked-list is the main method of iterating the listeners. The
    //  nodes are the map's values, so their addresses are stable, and the
    //  linked-list pointers are formed within them.
    //
    //  Since the map is the backing structure, adding or removing a listener
    //  is a single map node allocation/deallocation: there is never a window
    //  where the two structures can disagree, and the insert is strongly
    //  exception-safe without any try-catch (if the map's single allocation
    //  throws, nothing has been modified yet).
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

        //  Linked-list pointers, formed within the map's values.
        Node* next = nullptr;
        Node* prev = nullptr;

#ifdef PA_DEBUG_ListenerSet
        LifetimeSanitizer sanitizer;
#endif

        Node(ListenerType& p_listener)
            : listener(p_listener)
#ifdef PA_DEBUG_ListenerSet
            , sanitizer("Node")
#endif
        {}
    };

    //  Shared implementation of add()/push_front()/push_back(). Inserts
    //  "listener" at the front or back of the call order. If the listener is
    //  already present, this is a no-op and the call order is not changed.
    void insert(ListenerType& listener, bool front, const char* label);

    //  Unlink "node" from the linked-list. The caller must hold "m_lock" and
    //  "node.lock" (write). The map entry is left in place.
    void unlink_node(Node& node);


private:
    //  Optimization. Keep an atomic version of the count. This will let us
    //  skip the lock when there are no listeners.
    std::atomic<size_t> m_count;

    const bool m_suppress_lock_prints;
    mutable SpinLock m_lock;

    Node* m_head = nullptr;
    Node* m_tail = nullptr;
    std::map<ListenerType*, Node> m_listeners;

#ifdef PA_DEBUG_ListenerSet
    LifetimeSanitizer m_sanitizer;
#endif
};





template <typename ListenerType>
ListenerSet<ListenerType>::ListenerSet(bool suppress_lock_prints)
    : m_suppress_lock_prints(suppress_lock_prints)
{}

template <typename ListenerType>
void ListenerSet<ListenerType>::insert(ListenerType& listener, bool front, const char* label){
#ifdef PA_DEBUG_ListenerSet
    auto scope = m_sanitizer.check_scope();
#endif
    WriteSpinLock lg(m_lock, m_suppress_lock_prints ? nullptr : label);

    //  This is the only allocation for the whole operation. If it throws, the
    //  map is unchanged and the linked-list pointers have not been touched
    //  yet, so the structure is left consistent (strong exception safety).
    auto ret = m_listeners.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(&listener),
        std::forward_as_tuple(listener)
    );
    if (!ret.second){
        return;
    }

    Node& node = ret.first->second;

#ifdef PA_DEBUG_ListenerSet
    node.sanitizer.check_usage();
#endif
    if (front){
        node.prev = nullptr;
        node.next = m_head;
        if (m_head != nullptr){
#ifdef PA_DEBUG_ListenerSet
            m_head->sanitizer.check_usage();
#endif
            m_head->prev = &node;
        }else{
            m_tail = &node;
        }
        m_head = &node;
    }else{
        node.prev = m_tail;
        node.next = nullptr;
        if (m_tail != nullptr){
#ifdef PA_DEBUG_ListenerSet
            m_tail->sanitizer.check_usage();
#endif
            m_tail->next = &node;
        }else{
            m_head = &node;
        }
        m_tail = &node;
    }
    m_count.store(m_listeners.size(), std::memory_order_release);
}

template <typename ListenerType>
void ListenerSet<ListenerType>::push_front(ListenerType& listener){
    insert(listener, true, "ListenerSet::push_front()");
}
template <typename ListenerType>
void ListenerSet<ListenerType>::push_back(ListenerType& listener){
    insert(listener, false, "ListenerSet::push_back()");
}
template <typename ListenerType>
void ListenerSet<ListenerType>::add(ListenerType& listener){
    insert(listener, false, "ListenerSet::add()");
}

template <typename ListenerType>
void ListenerSet<ListenerType>::unlink_node(Node& node){
    if (node.prev != nullptr){
#ifdef PA_DEBUG_ListenerSet
        node.prev->sanitizer.check_usage();
#endif
        node.prev->next = node.next;
    }else{
        m_head = node.next;
    }
    if (node.next != nullptr){
#ifdef PA_DEBUG_ListenerSet
        node.next->sanitizer.check_usage();
#endif
        node.next->prev = node.prev;
    }else{
        m_tail = node.prev;
    }
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

        Node& node = iter->second;

#ifdef PA_DEBUG_ListenerSet
        node.sanitizer.check_usage();
#endif

        if (!node.lock.try_acquire_write()){
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

        unlink_node(node);

#ifdef PA_DEBUG_ListenerSet
        node.sanitizer.check_usage();
#endif

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

    Node& node = iter->second;
    if (!node.lock.try_acquire_write()){
        try{
            std::cout << "ListenerSet::try_remove(): Fail inner." << std::endl;
        }catch (...){}
        m_lock.unlock_write();
        return false;
    }

#ifdef PA_DEBUG_ListenerSet
    node.sanitizer.check_usage();
#endif

    unlink_node(node);

#ifdef PA_DEBUG_ListenerSet
    node.sanitizer.check_usage();
#endif

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

    Node* node = m_head;
    while (node != nullptr){
        {
            ReadSpinLock lg(node->lock, m_suppress_lock_prints ? nullptr : "ListenerSet::run_method()");

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

    Node* node = m_head;
    while (node != nullptr){
        bool return_now = false;
        {
            ReadSpinLock lg(node->lock, m_suppress_lock_prints ? nullptr : "ListenerSet::run_on_all()");

#ifdef PA_DEBUG_ListenerSet
            node->sanitizer.check_usage();
#endif
            m_lock.unlock_read();
            try{
                return_now = lambda(node->listener);
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
        node->sanitizer.check_usage();
#endif
        node = node->next;
    }

    m_lock.unlock_read();
    if (err){
        std::rethrow_exception(err);
    }
}

void add_tests_ListenerSet(UnitTestDatabase& database);




}
#endif
