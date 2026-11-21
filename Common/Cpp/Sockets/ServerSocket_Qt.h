/*  Server Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The Qt backend of the per-connection AbstractServerSocket, for
 *  callers that need to integrate with a Qt event loop.
 *
 *  The QTcpServer (and its connection sockets) live on the global Qt
 *  event thread. The server hands each accepted client out as a
 *  ServerSocketConnection_Qt object: a plain C++ shell whose send()/
 *  close() calls run on the event thread (inline when already on it,
 *  queued otherwise). The shell never owns a QObject, so its destructor
 *  is safe from any thread even after the server (and the event-thread
 *  objects) are gone - the connection only becomes inert.
 *
 */

#ifndef PokemonAutomation_ServerSocket_Qt_H
#define PokemonAutomation_ServerSocket_Qt_H

#include <chrono>
#include <algorithm>
#include <memory>
#include <string>
#include <atomic>
#include <vector>
#include <QObject>
#include <QSharedPointer>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "AbstractServerSocket.h"

class QTcpServer;
class QTcpSocket;

namespace PokemonAutomation{


class ThreadPool;



class ServerSocket_Qt;


class ServerSocketConnection_Qt final
    : public AbstractServerSocketConnection
    , public std::enable_shared_from_this<ServerSocketConnection_Qt>
{
public:
    //  Carries shared ownership (the signals pass it by value) so that a
    //  call still queued when the caller times out, or when
    //  "close_and_wait()" destroys the server and Qt discards the pending
    //  call, keeps the payload - and the connection shell - alive exactly
    //  as long as the event itself: the handler's copy outlives a
    //  timed-out caller, and a discarded event leaves the caller's copy
    //  as the last reference.
    struct SendData{
        std::string packet;
        //  The event-thread write loop stops at this deadline, so a send
        //  that can never complete (a client that never reads) cannot pin
        //  the server's event thread forever and block
        //  "close_and_wait()". "max()" means no deadline (the infinite
        //  default).
        std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
        //  The running byte count: the write loop updates it as it goes,
        //  so a close that cancels the send mid-write can report the
        //  bytes written so far (best effort).
        size_t bytes_sent = 0;
        //  Set when the connection is closed or the server is torn down;
        //  the queued write loop then finishes without writing.
        bool cancelled = false;
        //  Set when the event-thread write loop finishes or when the send
        //  is cancelled.
        bool done = false;
        //  The connection this send belongs to; the write loop writes to
        //  that connection's socket, so a send can never reach a
        //  different connection's socket. The shared ownership keeps the
        //  shell alive until the queued call runs (or is discarded with
        //  the server).
        std::shared_ptr<ServerSocketConnection_Qt> conn;
        Mutex lock;
        ConditionVariable cv;
    };

    virtual bool is_open() const override{
        return m_open.load(std::memory_order_relaxed);
    }

    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout
    ) override;

    virtual void close() override;


private:
    friend class ServerSocket_Qt;

    //  Backend internals: the server's event thread and the shell's own
    //  send()/close() only.

    //  Constructed on the server's event thread (in
    //  "on_new_connection()"), with the accepted socket.
    ServerSocketConnection_Qt(ServerSocket_Qt& server, QTcpSocket* sock)
        : m_server(&server)
        , m_sock(sock)
    {}

    //  The accepted socket. Only the event thread reads/writes it.
    QTcpSocket* socket() const{
        return m_sock;
    }
    bool close_requested() const{
        return m_close_requested.load(std::memory_order_relaxed);
    }
    //  Marks the connection closed without touching the socket. The
    //  event thread calls this when the peer goes away, so
    //  is_open()/send() report the death.
    void mark_closed(){
        m_open.store(false, std::memory_order_release);
    }
    //  Flips the connection closed and drains the in-flight sends,
    //  waking their callers with the bytes written so far (best
    //  effort). "m_lock" must already be held: it is what keeps a
    //  concurrent "send()" from registering after the drain, and it is
    //  what the server's teardown takes before it can destroy the
    //  server - so a "send()"/"close()" that got past its m_open check
    //  can never reach a freed server.
    void drain_inflight_sends(){
        const std::vector<QSharedPointer<SendData>> cancelled = std::move(m_inflight);
        for (const auto& send_data : cancelled){
            {
                std::lock_guard<Mutex> lg(send_data->lock);
                send_data->cancelled = true;
                send_data->done = true;
            }
            send_data->cv.notify_all();
        }
    }
    //  Destructor path: the server is going away, so mark the connection
    //  closed as application-initiated (a later "close()" is a no-op and
    //  never touches the dead server) and cancel the in-flight sends. The
    //  socket itself is destroyed with the QTcpServer.
    void close_for_teardown();
    //  Runs on the event thread (from the server's write loop): the send
    //  finished; remove it from the in-flight list and wake its caller.
    void finish_send(QSharedPointer<SendData> send_data, size_t bytes_sent){
        {
            std::lock_guard<RecursiveMutex> lg(m_lock);
            //  "close_for_teardown()"/"close()" may already have claimed
            //  this send; only remove it if it still owns its slot.
            m_inflight.erase(
                std::remove(
                    m_inflight.begin(),
                    m_inflight.end(),
                    send_data
                ),
                m_inflight.end()
            );
        }
        {
            std::lock_guard<Mutex> lg(send_data->lock);
            send_data->bytes_sent = bytes_sent;
            send_data->done = true;
        }
        send_data->cv.notify_all();
    }

    //  Raw pointer to the server, dereferenced only while holding
    //  "m_lock": the server's teardown takes that same lock before it
    //  destroys the server, so "send()"/"close()" can never reach a
    //  freed server, and after teardown "m_open"/"m_close_requested"
    //  make them return before they would.
    ServerSocket_Qt* m_server;
    //  Owned by Qt (a child of the QTcpServer); destroyed on the event
    //  thread. Never dereferenced after the connection closed.
    QTcpSocket* m_sock;
    std::atomic<bool> m_open{true};
    std::atomic<bool> m_close_requested{false};
    //  Guards the "m_inflight" list, the "m_open" flips, and the
    //  "m_close_requested" claim. "send()" holds it from its m_open check
    //  through the queued emit, and "close()"/"close_for_teardown()" hold
    //  it for their whole close, so a send either registers before the
    //  close's drain (and gets cancelled, waking its wait) or sees the
    //  closed connection and returns without queueing anything - and
    //  neither one can queue into a server the close already freed.
    //  It is recursive because the emit runs the handler inline when the
    //  caller is the event thread (AutoConnection), and "on_send"/"on_
    //  close_connection" re-take it on that path.
    RecursiveMutex m_lock;
    std::vector<QSharedPointer<SendData>> m_inflight;
};



class ServerSocket_Qt final : public QObject, public AbstractServerSocket{
    Q_OBJECT

public:
    ServerSocket_Qt(ThreadPool&){}

    virtual ~ServerSocket_Qt(){
        close_and_wait();
    }

    //  Binds to "address" and listens on "port" on the Qt event thread.
    //  This blocks until the QTcpServer has been created on that thread.
    virtual void start(const std::string& address, uint16_t port) override;

    virtual ConnectionPtr active_connection() override{
        std::lock_guard<Mutex> lg(m_lock);
        return m_active;
    }


signals:
    void internal_send(QSharedPointer<ServerSocketConnection_Qt::SendData> send_data);
    void internal_close_connection();


private:
    friend class ServerSocketConnection_Qt;

    //  The backend's internal state machine; not part of the application
    //  API (the listeners and active_connection() are the observables).
    enum class State{
        NOT_RUNNING,
        LISTENING,
        CONNECTED,
        DESTRUCTING,
    };
    std::atomic<State> m_state{State::NOT_RUNNING};

    //  Queues a connection's send on the event thread (or runs it inline
    //  when called from that thread). Called from the shell's "send()"
    //  with the shell's lock held, which the write loop's
    //  "finish_send()" re-takes on that inline path - hence the
    //  recursive shell lock.
    void queue_send(QSharedPointer<ServerSocketConnection_Qt::SendData> send_data){
        emit internal_send(send_data);
    }

    //  Queues the active connection's close on the event thread (or runs
    //  it inline when called from that thread). Called from the shell's
    //  "close()" with the shell's lock held. "close()" does not wait for
    //  the abort: "state()"/"active_connection()" update when the call
    //  runs.
    void close_connection_on();

    //  Runs on the QTcpServer's event thread.
    void on_new_connection();

    //  The active connection ended, or was closed by "close()". Only the
    //  first of the (errorOccurred, disconnected) signals for a given
    //  connection fires a callback, and "on_disconnect" is not fired for
    //  a connection closed by "close()" or "close_and_wait()".
    //  Runs on the QTcpServer's event thread.
    void connection_lost(QTcpSocket* sock, const std::string& error_message);

    //  Runs on the QTcpServer's event thread.
    void on_close_connection();

    //  Runs on the QTcpServer's event thread. The context object is the
    //  server, so the call still runs if the connection socket has
    //  already been aborted, and it just reports 0 bytes in that case.
    void on_send(QSharedPointer<ServerSocketConnection_Qt::SendData> send_data);

    //  Stops the server: it destroys the QTcpServer (and its connection
    //  socket) on the event thread, closes the active connection, and
    //  returns the state to State::NOT_RUNNING, so the object is fully
    //  torn down. It blocks until the QTcpServer has been destroyed, so
    //  any in-flight listener callback is finished by the time it
    //  returns; it is private and only called by the destructor.
    void close_and_wait() noexcept;

    std::unique_ptr<QTcpServer> make_server(const std::string& address, uint16_t port);

    Mutex m_lock;
    ConditionVariable m_cv;

    QTcpServer* m_server = nullptr;
    bool m_starting = false;

    //  The connection currently being served, or empty. The server keeps
    //  its reference for the connection's life; listeners keep their
    //  own, so the connection object outlives the server (inert).
    ConnectionPtr m_active;

    //  Written on the event thread by "make_server()", read after
    //  "add_object()" has returned.
    std::string m_listen_error;
};



}
#endif
