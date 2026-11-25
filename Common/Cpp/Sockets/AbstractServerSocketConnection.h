/*  Server Socket Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A single client connection on an AbstractServerSocket.
 *
 *  The server hands these out through its listeners, one per accepted
 *  client. All I/O happens on the connection object, not on the server:
 *  a write that outlives its connection belongs to this object's dead
 *  socket and can never reach a later client's socket.
 *
 *  Ownership is shared: the server keeps a reference for as long as
 *  the connection is active, and listeners keep their own, so the
 *  object outlives the server. Once the connection has closed (by
 *  either side, or when the server is destroyed), the object remains
 *  valid but inert: "is_open()" is false, "send()" returns 0, and
 *  "close()" is a no-op. Implementations must keep it that way: the
 *  connection's resources belong to the server, and the last
 *  reference only ever destroys this inert shell, from any thread.
 *
 *  The transport contract is best-effort (like raw sockets): closing a
 *  connection whose peer is still sending may reset it, so bytes that
 *  earlier "send()" calls accepted are not guaranteed to reach the
 *  peer, and the peer's unread data is discarded.
 *
 */

#ifndef PokemonAutomation_Sockets_AbstractServerSocketConnection_H
#define PokemonAutomation_Sockets_AbstractServerSocketConnection_H

#include <chrono>
#include <cstddef>

namespace PokemonAutomation{



class AbstractServerSocketConnection{
public:
    AbstractServerSocketConnection() = default;
    virtual ~AbstractServerSocketConnection() = default;

    //  True while the connection is open. This is a hint, not a
    //  synchronization point: the connection can close at any moment,
    //  even after this returned true, so rely on "send()" (which
    //  returns 0 once the connection is closed) for liveness checks.
    virtual bool is_open() const = 0;

    //  Thread-safe. Sends "bytes" of "data" to the peer and returns
    //  the number of bytes the transport accepted. "data" must stay
    //  valid until the call returns.
    //
    //  If the connection closes while the call is in flight, the call
    //  returns promptly with a best-effort count: 0 if the connection
    //  was already closed, otherwise an unspecified number of bytes
    //  that may or may not reach the peer. Treat a short return as
    //  "the connection is no longer usable", not as a progress report.
    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    ) = 0;

    //  Closes the connection; the server keeps listening for new
    //  clients. The peer may still be sending: its unread data is
    //  discarded, the close may be a TCP reset, and bytes accepted by
    //  earlier "send()" calls may be lost. In-flight "send()" calls
    //  return promptly with a best-effort count (see "send()").
    //
    //  Thread-safe. Safe to call from within a listener callback, and
    //  safe to call more than once (or after the connection has
    //  already closed): extra calls are no-ops. The object remains
    //  valid afterwards; it simply becomes inert.
    virtual void close() = 0;
};



}
#endif
