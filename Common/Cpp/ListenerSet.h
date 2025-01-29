/*  Listener Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ListenerSet_H
#define PokemonAutomation_ListenerSet_H

#include <map>
#include <mutex>

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



template <typename ListenerType>
class ListenerSet{
public:
    bool empty() const{
        std::lock_guard<std::mutex> lg(m_lock);
        return m_listeners.empty();
    }
    size_t count_unique() const{
        std::lock_guard<std::mutex> lg(m_lock);
        return m_listeners.size();
    }

    void add(ListenerType& listener){
        std::lock_guard<std::mutex> lg(m_lock);
        m_listeners[&listener]++;
    }
    void remove(ListenerType& listener){
        std::lock_guard<std::mutex> lg(m_lock);
        auto iter = m_listeners.find(&listener);
        if (iter == m_listeners.end()){
            return;
        }
        if (--iter->second == 0){
            m_listeners.erase(iter);
        }
    }

    template <typename Lambda>
    void run_lambda_unique(Lambda&& lambda){
        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            lambda(*item.first);
        }
    }
    template <typename Lambda>
    void run_lambda_with_duplicates(Lambda&& lambda){
        std::lock_guard<std::mutex> lg(m_lock);
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
        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            (item.first->*function)(std::forward<Args>(args)...);
        }
    }
    template <typename Function, class... Args>
    void run_method_with_duplicates(Function function, Args&&... args){
        std::lock_guard<std::mutex> lg(m_lock);
        for (auto& item : m_listeners){
            ListenerType& listener = *item.first;
            size_t count = item.second;
            do{
                (listener->*function)(std::forward<Args>(args)...);
            }while (--count);
        }
    }

private:
    mutable std::mutex m_lock;
    std::map<ListenerType*, size_t> m_listeners;
};




}
#endif
