/*  Abstract Server Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A minimal TCP server with per-connection objects.
 *
 *  It listens on a port and hands each accepted client to its
 *  listeners as an AbstractServerSocketConnection. The server has no
 *  send/receive of its own: all I/O happens on the connection objects
 *  it hands out, so a write that outlives its connection can never
 *  reach a later client's socket.
 *
 *  It handles one connection at a time: a client that connects while
 *  one is active is rejected.
 *
 *  Lifecycle: destroying the server stops it and closes every live
 *  connection. The shared ownership only guarantees the connection
 *  object's validity: after the server is gone, any use of the object
 *  reports the closed connection (is_open() is false, send() returns
 *  0, close() is a no-op).
 *
 *  Thread safety: "start()", "active_connection()",
 *  "add_listener()" and "remove_listener()" are safe to call from any
 *  thread. Listener callbacks may be called from any of the backend's
 *  worker threads and may overlap in time; it is always safe to call
 *  "send()"/"close()" on the passed connection from within a
 *  callback. A listener must not remove itself from inside its own
 *  callback ("remove_listener()" would block until the callback
 *  returns).
 *
 */

#ifndef PokemonAutomation_Sockets_AbstractServerSocket_H
#define PokemonAutomation_Sockets_AbstractServerSocket_H

#include <cstdint>
#include <memory>
#include <string>
#include "Common/Cpp/ListenerSet.h"
#include "AbstractServerSocketConnection.h"

namespace PokemonAutomation{



class AbstractServerSocket{
public:
    using ConnectionPtr = std::shared_ptr<AbstractServerSocketConnection>;

    struct Listener{
        //  Called when "start()" finishes.
        //  If successful, "error_message" is empty.
        //  Otherwise, it contains the error message.
        virtual void on_listen_finished(const std::string& error_message){}

        //  A new client connected. Keep "connection" (or a copy of it)
        //  to send on or close it later: it stays valid until the
        //  connection closes, even if this server is destroyed in the
        //  meantime (see AbstractServerSocketConnection).
        virtual void on_connect(ConnectionPtr connection){}

        //  A new client was rejected because a connection is already
        //  active.
        virtual void on_connect_rejected(){}

        //  Data arrived on "connection". "data" is only valid for the
        //  duration of this call.
        virtual void on_receive_data(ConnectionPtr connection, const void* data, size_t bytes){}

        //  "connection" was closed by the peer, or the transport
        //  failed. Not called for a connection the application closed
        //  itself with "close()". "error_message" is empty for a clean
        //  peer close.
        virtual void on_disconnect(ConnectionPtr connection, const std::string& error_message){}
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }


public:
    AbstractServerSocket() = default;
    virtual ~AbstractServerSocket() = default;

    //  Binds to "address" and listens on "port".
    //  Use "0.0.0.0" to accept connections on all interfaces.
    //  Only acts while the server is not running and no start attempt is
    //  still in flight; otherwise it is a no-op. A start that fails (for
    //  example, the port is busy) leaves the server not running, so it
    //  may be retried. The server stops only when it is destroyed.
    virtual void start(const std::string& address, uint16_t port) = 0;

    //  The connection currently being served, or nullptr when none is
    //  active. The pointer is valid for the life of that connection
    //  (see AbstractServerSocketConnection), not just while this call
    //  is running.
    virtual ConnectionPtr active_connection() = 0;


protected:
    ListenerSet<Listener> m_listeners;
};



}
#endif
