/*  Abstract Client Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Sockets_AbstractClientSocket_H
#define PokemonAutomation_Sockets_AbstractClientSocket_H

#include <string>
#include "Common/Compiler.h"
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
        //  Called at the start of the receiver thread. This can be used to set
        //  the thread priority of the thread.
        virtual void on_thread_start(){}

        //  Called when a connection attempt finishes.
        //  If successful, "error_message" is empty.
        //  Otherwise, it contains the error message.
        virtual void on_connect_finished(const std::string& error_message){}

        //  Called when the socket receives data from the server.
        virtual void on_receive_data(const void* data, size_t bytes){}
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.add(listener);
    }


public:
    AbstractClientSocket()
        : m_state(State::NOT_RUNNING)
    {}
    virtual ~AbstractClientSocket() = default;

    State state() const{
        return m_state.load(std::memory_order_relaxed);
    }

    virtual void close() noexcept = 0;
    virtual void connect(const std::string& address, uint16_t port) = 0;

    virtual size_t send(const void* data, size_t bytes) = 0;


protected:
    std::atomic<State> m_state;
    ListenerSet<Listener> m_listeners;
};





}
#endif
