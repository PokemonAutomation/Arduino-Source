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

#include <chrono>
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
        //  Called when "start()" finishes.
        //  If successful, "error_message" is empty.
        //  Otherwise, it contains the error message.
        virtual void on_listen_finished(const std::string& error_message){}
        virtual void on_connect(){}
        virtual void on_connect_rejected(){}
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
    AbstractServerSocket()
        : m_state(State::NOT_RUNNING)
    {}
    virtual ~AbstractServerSocket() = default;

    State state() const{
        return m_state.load(std::memory_order_relaxed);
    }

    //  Binds to "address" and listens on "port".
    //  Use "0.0.0.0" to accept connections on all interfaces.
    virtual void start(const std::string& address, uint16_t port) = 0;

    //  Close the connection and immediately cancel all ongoing sends.
    //  The socket will continue listening for a new incoming connection.
    virtual void close_connection() = 0;

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
