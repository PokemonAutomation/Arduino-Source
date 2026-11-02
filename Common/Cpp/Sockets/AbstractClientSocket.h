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
        //  Called at the start of the receiver thread. This can be used to set
        //  the thread priority of the thread.
        virtual void on_thread_start(){}

        //  Called when a connection attempt finishes.
        //  If successful, "error_message" is empty.
        //  Otherwise, it contains the error message.
        virtual void on_connect_finished(const std::string& error_message){}

        //  Called when the socket receives data from the server.
        virtual void on_receive_data(const void* data, size_t bytes){}

        //  Called when an established connection is lost (the server closed
        //  the connection or a socket error occurred). It is not called by
        //  "close()".
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

    virtual void close() noexcept = 0;
    virtual void connect(const std::string& address, uint16_t port) = 0;

    //  Sends data on the active connection. This blocks until every byte
    //  has been written, the connection is lost, or "timeout" elapses.
    //  "std::chrono::milliseconds::max()" (the default) waits indefinitely.
    //  Returns the number of bytes actually written, which may be less
    //  than "bytes" if the deadline elapsed or the connection was lost.
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
