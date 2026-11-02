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

    //  Stops the connection and returns the socket to State::NOT_RUNNING
    //  so "connect()" can be called again. This is safe to call multiple
    //  times.
    //
    //  It cancels any in-flight operations: no further
    //  "Listener::on_receive_data" callbacks are started (one already
    //  running is allowed to finish), and an in-flight "send()" returns
    //  promptly with the number of bytes actually written so far (best
    //  effort). A "send()" that is already racing "close()" may still
    //  complete.
    //
    //  This blocks until the receive thread has exited, so it must not be
    //  called from within a listener callback.
    virtual void close() noexcept = 0;
    virtual void connect(const std::string& address, uint16_t port) = 0;

    //  Sends data on the active connection. This blocks until every byte
    //  has been written, the connection is lost, "close()" is called, or
    //  "timeout" elapses. "std::chrono::milliseconds::max()" (the default)
    //  waits indefinitely. Returns the number of bytes actually written,
    //  which may be less than "bytes" if the deadline elapsed, the
    //  connection was lost, or "close()" cancelled the send. If there is
    //  no active connection, returns 0.
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
