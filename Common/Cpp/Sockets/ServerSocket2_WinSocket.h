/*  Server Socket 2 (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The WinSocket backend of the per-connection AbstractServerSocket2.
 *
 *  One worker thread (from the thread pool) runs the accept loop for the
 *  whole lifetime of the server, and one worker thread per accepted
 *  connection. Each accepted client becomes a
 *  ServerSocket2Connection_WinSocket object, which owns the socket's I/O
 *  (send()/close()) and outlives the server as an inert object.
 *
 */

#ifndef PokemonAutomation_ServerSocket2_WinSocket_H
#define PokemonAutomation_ServerSocket2_WinSocket_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <winsock2.h>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "AbstractServerSocket2.h"

namespace PokemonAutomation{


class ThreadPool;


class ServerSocket2Connection_WinSocket final : public AbstractServerSocketConnection{
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
    friend class ServerSocket2_WinSocket;

    //  Backend internals: the server's accept/connection threads only.

    //  Constructed by the server on accept, with the accepted socket.
    explicit ServerSocket2Connection_WinSocket(SOCKET client_socket)
        : m_client(client_socket)
    {}

    SOCKET socket() const{
        return m_client;
    }
    bool close_requested() const{
        return m_close_requested.load(std::memory_order_relaxed);
    }
    //  Marks the connection closed without touching the socket. The
    //  connection thread calls this when the peer goes away, so
    //  is_open()/send() report the death.
    void mark_closed(){
        m_open.store(false, std::memory_order_release);
    }

    SOCKET m_client;
    std::atomic<bool> m_open{true};
    std::atomic<bool> m_close_requested{false};
    //  Serializes logical sends: two concurrent send() calls must not
    //  interleave their bytes on the wire. close()/mark_closed() do not
    //  take it, so a slow send can never block the close.
    Mutex m_send_lock;
};



class ServerSocket2_WinSocket final : public AbstractServerSocket2{
public:

    ServerSocket2_WinSocket(ThreadPool& thread_pool);

    virtual ~ServerSocket2_WinSocket(){
        close_and_wait();
    }

    virtual void start(const std::string& address, uint16_t port) override;

    virtual ConnectionPtr active_connection() override{
        std::lock_guard<Mutex> lg(m_lock);
        return m_active;
    }


private:

    static void ensure_wsa_started();

    //  Stops the server: it stops accepting new connections, closes the
    //  active connection, and returns the server to State::NOT_RUNNING so
    //  the object is fully torn down. It blocks until the worker threads
    //  have exited, so it is private and only called by the destructor.
    void close_and_wait() noexcept;

    void accept_thread_loop(std::string address, uint16_t port);
    void connection_thread_loop(std::shared_ptr<ServerSocket2Connection_WinSocket> conn);

    //  A start attempt failed (or the server was torn down while the task
    //  was still starting up): mark the task dead so that a retrying
    //  start() may dispatch a new one, and report the result to the
    //  listeners.
    void listen_failed(const std::string& error);


private:
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

    //  Protects m_accept_threads and m_client_thread. Kept separate from
    //  m_lock because the task's lifetime interacts with it: joining a
    //  task waits for it to exit, and exiting requires m_lock. Holding
    //  m_lock while a task slot is destroyed would deadlock, so slot
    //  operations happen under m_task_lock with m_lock released.
    Mutex m_task_lock;

    SOCKET m_listen = INVALID_SOCKET;

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
    AsyncTask m_client_thread;
};



}
#endif
