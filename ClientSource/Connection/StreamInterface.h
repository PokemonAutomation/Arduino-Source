/*  Stream Connection Interface
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_StreamInterface_H
#define PokemonAutomation_StreamInterface_H

#include <mutex>
#include <set>

namespace PokemonAutomation{


class StreamListener{
public:
    virtual void on_recv(const void* data, size_t bytes) = 0;
};


class StreamConnection{
public:
    virtual ~StreamConnection(){}
    virtual void stop() = 0;

    void add_listener(StreamListener& listener){
        std::lock_guard<std::mutex> lg(m_listener_lock);
        m_listeners.insert(&listener);
    }
    void remove_listener(StreamListener& listener){
        std::lock_guard<std::mutex> lg(m_listener_lock);
        m_listeners.erase(&listener);
    }

    virtual void send(const void* data, size_t bytes) = 0;

protected:
    void on_recv(const void* data, size_t bytes){
        std::lock_guard<std::mutex> lg(m_listener_lock);
        for (StreamListener* listener : m_listeners){
            listener->on_recv(data, bytes);
        }
    }

protected:
    std::mutex m_listener_lock;
    std::set<StreamListener*> m_listeners;
};




}

#endif
