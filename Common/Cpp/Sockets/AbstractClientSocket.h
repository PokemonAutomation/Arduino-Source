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
        //  A listener must outlive the socket that calls it. Callbacks can
        //  still be delivered after a non-blocking "close()" returns - the
        //  socket's threads only stop when the socket itself is destroyed -
        //  so a listener must not be destroyed before the socket that added
        //  it. Declare the listener before the socket, so that it is
        //  destroyed after the socket.
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

    //  Requests immediate teardown of the active connection, without waiting
    //  for the receive thread to exit. This is safe to call from within a
    //  listener callback (which runs on that thread; the blocking teardown
    //  there would deadlock). It marks the socket DESTRUCTING and releases
    //  the underlying socket; the receive thread observes this on its next
    //  iteration and exits. It cancels any in-flight "send()": its caller
    //  returns promptly with the number of bytes actually written so far
    //  (best effort). No "on_disconnect" callback is fired. The socket is
    //  left in State::DESTRUCTING; the socket's destructor completes the
    //  teardown (joins the thread and returns it to NOT_RUNNING), so the
    //  socket cannot be "connect()"ed again - destroy it and use a new one.
    //  Calling "close()" when there is no active connection is a no-op. It
    //  is safe to call multiple times.
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
