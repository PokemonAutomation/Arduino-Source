/*  Abstract Server Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A minimal TCP server: it listens on a port and handles one connection
 *  at a time. The implementation uses one worker thread for the accept
 *  loop and one worker thread (from the thread pool) per active
 *  connection.
 *
 */

#ifndef PokemonAutomation_Sockets_AbstractServerSocket_H
#define PokemonAutomation_Sockets_AbstractServerSocket_H

#include <string>
#include "Common/Cpp/ListenerSet.h"

namespace PokemonAutomation{



class AbstractServerSocket{
public:
    enum class State{
        NOT_RUNNING,
        LISTENING,
        CONNECTED,
        DESTRUCTING,
    };

    struct Listener{
        //  Called at the start of the thread that will handle a connection.
        //  This can be used to set the thread priority of the thread.
        virtual void on_thread_start(){}

        //  Called when "start()" finishes.
        //  If successful, "error_message" is empty.
        //  Otherwise, it contains the error message.
        virtual void on_listen_finished(const std::string& error_message){}

        //  Called when a client connects.
        virtual void on_connect(){}

        //  Called when an incoming connection was rejected because a
        //  client is already connected.
        virtual void on_connect_rejected(){}

        //  Called when the server receives data from the client.
        virtual void on_receive_data(const void* data, size_t bytes){}

        //  Called when the active connection is lost (the client closed
        //  the connection or a socket error occurred). It is not called
        //  by "close()" or "close_connection()".
        virtual void on_disconnect(const std::string& error_message){}
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }


public:
    AbstractServerSocket()
        : m_state(State::NOT_RUNNING)
    {}
    virtual ~AbstractServerSocket() = default;

    State state() const{
        return m_state.load(std::memory_order_relaxed);
    }

    //  Binds to "address" and listens on "port". Use "0.0.0.0" to accept
    //  connections on all interfaces. The result is reported through
    //  "Listener::on_listen_finished", after which the state is
    //  State::LISTENING on success or State::NOT_RUNNING on failure.
    virtual void start(const std::string& address, uint16_t port) = 0;

    //  Closes the active connection, if any, and keeps listening.
    virtual void close_connection() = 0;

    //  Stops the server: it stops accepting new connections, closes the
    //  active connection, and returns the socket to State::NOT_RUNNING so
    //  "start()" can be called again. This is safe to call multiple times.
    //
    //  This blocks until the worker threads have exited, so it must not be
    //  called from within a listener callback (which runs on one of those
    //  threads).
    virtual void close() noexcept = 0;

    //  Sends data to the active connection.
    //  If there is no active connection, returns 0.
    virtual size_t send(const void* data, size_t bytes) = 0;


protected:
    std::atomic<State> m_state;
    ListenerSet<Listener> m_listeners;
};



}
#endif
