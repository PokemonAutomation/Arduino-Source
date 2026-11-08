/*  Abstract Client Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Sockets_AbstractClientSocket_H
#define PokemonAutomation_Sockets_AbstractClientSocket_H

#include <chrono>
#include <string>
#include "Common/Cpp/ListenerSet.h"

namespace PokemonAutomation{



class AbstractClientSocket{
public:
    enum class State{
        NOT_RUNNING,
        CONNECTING,
        CONNECTED,
        DESTRUCTING,
    };

    struct Listener{
        virtual void on_connect_finished(const std::string& error_message){}
        virtual void on_receive_data(const void* data, size_t bytes){}
        virtual void on_disconnect(const std::string& error_message){}
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }


public:
    AbstractClientSocket()
        : m_state(State::NOT_RUNNING)
    {}
    virtual ~AbstractClientSocket() = default;

    State state() const{
        return m_state.load(std::memory_order_relaxed);
    }

    //  Close the connection and immediately cancel all ongoing sends.
    virtual void close() noexcept = 0;

    virtual void connect(const std::string& address, uint16_t port) = 0;

    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    ) = 0;


protected:
    std::atomic<State> m_state;
    ListenerSet<Listener> m_listeners;
};





}
#endif
