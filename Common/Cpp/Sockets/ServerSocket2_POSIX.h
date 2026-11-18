/*  Server Socket 2 (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The POSIX backend of the per-connection AbstractServerSocket2.
 *
 *  One worker thread (from the thread pool) runs the accept loop for the
 *  whole lifetime of the server, and one worker thread per accepted
 *  connection. Each accepted client becomes a
 *  ServerSocket2Connection_POSIX object, which owns the socket's I/O
 *  (send()/close()) and outlives the server as an inert object.
 *
 */

#ifndef PokemonAutomation_ServerSocket2_POSIX_H
#define PokemonAutomation_ServerSocket2_POSIX_H

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "AbstractServerSocket2.h"

namespace PokemonAutomation{


class ThreadPool;
class ServerSocket2_POSIX;


class ServerSocket2Connection_POSIX final : public AbstractServerSocketConnection{
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
    friend class ServerSocket2_POSIX;

    //  Backend internals: the server and this connection's own thread
    //  only.

    //  Constructed by the server on accept, with the accepted socket.
    ServerSocket2Connection_POSIX(ServerSocket2_POSIX& server, int client_socket)
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
    //  keeps the object alive while thread_loop() runs).
    void start_thread(ThreadPool& thread_pool){
        m_thread = thread_pool.dispatch([this]{
            thread_loop();
        });
    }
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

    //  The owning server. Dereferenced only from thread_loop(), which the
    //  server's close_and_wait() has joined before the server is
    //  destroyed, so it can never be used after the server's death.
    ServerSocket2_POSIX* m_server;

    int m_client;
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



class ServerSocket2_POSIX final : public AbstractServerSocket2{
public:
    ServerSocket2_POSIX(ThreadPool& thread_pool);

    virtual ~ServerSocket2_POSIX(){
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
    //  Hands the CONNECTED state back to the server so it can accept the
    //  next client. The m_active slot is kept: it anchors the connection's
    //  lifetime while its thread runs.
    void release_connection();

    //  A start attempt failed (or the server was torn down while the task
    //  was still starting up): mark the task dead so that a retrying
    //  start() may dispatch a new one, and report the result to the
    //  listeners.
    void listen_failed(const std::string& error);


private:
    friend class ServerSocket2Connection_POSIX;

    ThreadPool& m_thread_pool;
    Mutex m_lock;
    ConditionVariable m_cv;
    //  "m_starting" covers the window between start() claiming the start
    //  and the task being published in m_accept_threads; "m_task_alive"
    //  covers the task's whole life (until it fails, or
    //  "close_and_wait()" joins it). While either is set, start() is a
    //  no-op - so there is never more than one live start task, and a
    //  task that is finally scheduled can only be stale in the sense
    //  that "close_and_wait()" won the race (checked against the state
    //  when it publishes).
    bool m_starting = false;
    bool m_task_alive = false;
    int m_closing = 0;

    //  Protects m_accept_threads. Kept separate from m_lock because the
    //  task's lifetime interacts with it: joining a task waits for it to
    //  exit, and exiting requires m_lock. Holding m_lock while a task
    //  slot is destroyed would deadlock, so slot operations happen under
    //  m_task_lock with m_lock released. (The connection threads' tasks
    //  live in the connections themselves; close_and_wait() joins them
    //  through the connection.)
    Mutex m_task_lock;

    int m_listen = -1;

    //  The connection currently being served, or empty. The server keeps
    //  its reference for the connection's life; listeners keep their own,
    //  so the connection object outlives the server (inert).
    ConnectionPtr m_active;

    //  One task per start() call that actually dispatched (start() is a
    //  no-op while a previous task is still alive), so the list holds
    //  the failed attempts' tasks plus the one live task. start() must
    //  never destroy a previous task's AsyncTask here: its destructor
    //  blocks until that task has finished. close_and_wait() joins the
    //  whole list (the finished ones return immediately).
    std::vector<AsyncTask> m_accept_threads;
};



}
#endif
