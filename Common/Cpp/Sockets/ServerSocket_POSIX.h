/*  Server Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The POSIX backend of the per-connection AbstractServerSocket.
 *
 *  One worker thread (from the thread pool) runs the accept loop for the
 *  whole lifetime of the server, and one worker thread per accepted
 *  connection. Each accepted client becomes a
 *  ServerSocketConnection_POSIX object, which owns the socket's I/O
 *  (send()/close()) and outlives the server as an inert object.
 *
 */

#ifndef PokemonAutomation_ServerSocket_POSIX_H
#define PokemonAutomation_ServerSocket_POSIX_H

#include <atomic>
#include <memory>
#include <string>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "AbstractServerSocket.h"

namespace PokemonAutomation{


class ThreadPool;
class ServerSocket_POSIX;


class ServerSocketConnection_POSIX final : public AbstractServerSocketConnection{
public:
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
    friend class ServerSocket_POSIX;

    //  Backend internals: the server and this connection's own thread
    //  only.

    //  Constructed by the server on accept, with the accepted socket.
    ServerSocketConnection_POSIX(ServerSocket_POSIX& server, int client_socket)
        : m_server(&server)
        , m_client(client_socket)
    {}

    //  The connection's own thread (dispatched by the server's accept
    //  loop via start_thread()): reports the connection to the listeners,
    //  reads until the peer or the application closes it, and hands the
    //  state back. The server's bookkeeping (m_active/m_state) is reached
    //  through the server's report_*()/release_connection() methods; the
    //  report methods read m_active themselves, so this method only ever
    //  references "this" - which is safe, because the server drops its
    //  m_active reference only after it has joined this thread.
    void thread_loop();

    //  Dispatches the thread. The server calls this exactly once, right
    //  after it has published the connection in m_active (which is what
    //  keeps the object alive while thread_loop() runs). Defined in the
    //  .cpp: it needs the complete ThreadPool type.
    void start_thread(ThreadPool& thread_pool);
    //  Joins the thread. The server calls this before it drops its
    //  m_active reference - to replace the slot or to tear down - because
    //  m_active is what keeps the object alive while thread_loop() runs.
    void wait_thread(){
        m_thread.wait_and_ignore_exceptions();
    }

    //  Marks the connection closed without touching the socket. The
    //  server calls this when the connection thread could not be
    //  started, so is_open()/send() report the death.
    void mark_closed(){
        m_open.store(false, std::memory_order_release);
    }

    //  Takes ownership of the accepted socket, returning it, or -1 if
    //  it has already been taken. The socket is handed off exactly
    //  once - by close() (which then shuts it down, leaving the
    //  transport to close it) or by the transport/accept-loop paths
    //  (which close it straight away). This matters because the kernel
    //  reuses closed fd numbers immediately: once the transport has
    //  exited and closed its socket, a stale ::shutdown() or ::close()
    //  on the same number would act on whatever connection now owns
    //  it, not on this one.
    int take_fd(){
        return m_client.exchange(-1, std::memory_order_acq_rel);
    }

    //  The owning server. Dereferenced only from thread_loop(), which the
    //  server's close_and_wait() has joined before the server is
    //  destroyed, so it can never be used after the server's death.
    ServerSocket_POSIX* m_server;

    //  The accepted socket; -1 once take_fd() has handed it off.
    std::atomic<int> m_client;
    //  The task running thread_loop() (empty if the dispatch failed). It
    //  lives in the connection rather than the server, so the server's
    //  only handle to the connection is m_active.
    AsyncTask m_thread;
    std::atomic<bool> m_open{true};
    std::atomic<bool> m_close_requested{false};
    //  Serializes logical sends: two concurrent send() calls must not
    //  interleave their bytes on the wire. close()/mark_closed() do not
    //  take it, so a slow send can never block the close.
    Mutex m_send_lock;
};



class ServerSocket_POSIX final : public AbstractServerSocket{
public:
    ServerSocket_POSIX(ThreadPool& thread_pool);

    virtual ~ServerSocket_POSIX(){
        close_and_wait();
    }

    virtual void start(const std::string& address, uint16_t port) override;

    virtual ConnectionPtr active_connection() override{
        std::lock_guard<Mutex> lg(m_lock);
        return m_active;
    }


private:
    //  Stops the server: it stops accepting new connections, closes the
    //  active connection, and returns the server to State::NOT_RUNNING so
    //  the object is fully torn down. It blocks until the worker threads
    //  have exited, so it is private and only called by the destructor.
    void close_and_wait() noexcept;

    void accept_thread_loop(std::string address, uint16_t port);

    //  Called from a connection's thread (its thread_loop()); the
    //  connection does the I/O, the server does the listener dispatch
    //  and the state bookkeeping. They read the connection from m_active
    //  (which is still the calling connection for the whole life of the
    //  thread).
    void report_connect();
    void report_receive(const void* data, size_t bytes);
    void report_disconnect(const std::string& error);
    //  If "self" is still the active connection, hand its CONNECTED
    //  state back to the server. Called by the connection thread at
    //  exit, and by close() before the close becomes visible to the
    //  peer - a client reconnecting after it observes the close must
    //  not be rejected by a state that is still winding down. No-op
    //  otherwise (including for a connection that has already been
    //  replaced). The m_active slot is kept either way: it anchors the
    //  connection's lifetime while its thread runs, and the server drops
    //  the slot only after it has joined that thread.
    void release_connection(ServerSocketConnection_POSIX* self);

    //  A start attempt failed (or the server was torn down while the task
    //  was still starting up): mark the task dead so that a retrying
    //  start() may dispatch a new one, and report the result to the
    //  listeners.
    void listen_failed(const std::string& error);


private:
    friend class ServerSocketConnection_POSIX;

    //  The backend's internal state machine; not part of the application
    //  API (the listeners and active_connection() are the observables).
    enum class State{
        NOT_RUNNING,
        LISTENING,
        CONNECTED,
        DESTRUCTING,
    };
    std::atomic<State> m_state{State::NOT_RUNNING};

    ThreadPool& m_thread_pool;
    Mutex m_lock;
    ConditionVariable m_cv;
    //  "m_starting" covers the window between start() claiming the start
    //  and the task being published in m_accept_thread; "m_task_alive"
    //  covers the task's whole life (until it fails, or
    //  "close_and_wait()" joins it). While either is set, start() is a
    //  no-op - so there is never more than one live start task, and a
    //  task that is finally scheduled can only be stale in the sense
    //  that "close_and_wait()" won the race (checked against the state
    //  when it publishes).
    //  m_starting also serializes the writers of m_accept_thread: start()
    //  touches it only inside its m_starting window, and
    //  close_and_wait() waits for "!m_starting" before moving it out.
    bool m_starting = false;
    bool m_task_alive = false;
    int m_closing = 0;

    int m_listen = -1;

    //  The connection currently being served, or empty. The server keeps
    //  its reference for the connection's life; listeners keep their own,
    //  so the connection object outlives the server (inert).
    ConnectionPtr m_active;

    //  The start() task: the one live task plus, after a failed attempt,
    //  that attempt's finished task until the next start() replaces it.
    //  start() is a no-op while a task is alive, so a replaced task has
    //  already exited (its join at replacement is instant); the live one
    //  is joined by close_and_wait().
    AsyncTask m_accept_thread;
};



}
#endif
