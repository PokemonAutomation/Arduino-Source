/*  Server Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_POSIX_H
#define PokemonAutomation_ServerSocket_POSIX_H

#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "AbstractServerSocket.h"

namespace PokemonAutomation{

class ThreadPool;


class ServerSocket_POSIX final : public AbstractServerSocket{
public:

    ServerSocket_POSIX(ThreadPool& thread_pool);

    virtual ~ServerSocket_POSIX(){
        close_and_wait();
    }

    virtual void start(const std::string& address, uint16_t port) override;

    //  Closes the active connection and keeps listening. The connection
    //  thread stops and closes the socket itself; an in-flight "send()" is
    //  cancelled and returns promptly with the bytes written so far (the
    //  send loop re-checks the state every iteration). Safe to call from
    //  within a listener callback.
    virtual void close_connection() override;

    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max(),
        uint64_t connection = 0
    ) override;

    //  A token identifying the connection the socket is currently
    //  serving. It changes every time a new client is accepted, and it
    //  is 0 when no connection is active.
    virtual uint64_t connection_id() override;


private:

    //  Stops the server: it stops accepting new connections, closes the
    //  active connection, and returns the server to State::NOT_RUNNING so
    //  the object is fully torn down. It blocks until the worker threads
    //  have exited, so it is private and only called by the destructor.
    void close_and_wait() noexcept;

    void accept_thread_loop(std::string address, uint16_t port, uint64_t generation);
    void connection_thread_loop();

    //  "on_listen_finished" callback, but only for a task that still owns
    //  the server. A task whose generation is stale (a newer start() ran
    //  while it was still starting up) must stay silent, or its result
    //  would corrupt the attempt that replaced it.
    void report_listen_finished(uint64_t generation, const std::string& error);


private:
    ThreadPool& m_thread_pool;
    Mutex m_lock;
    ConditionVariable m_cv;
    bool m_starting = false;
    int m_closing = 0;

    //  Increments on every start(). The accept task captures its own
    //  generation so that a task left behind by a timed-out attempt can
    //  tell it was replaced when it finally gets scheduled.
    uint64_t m_generation = 0;

    //  Protects m_accept_threads and m_client_thread. Kept separate from
    //  m_lock because the task's lifetime interacts with it: joining a
    //  task waits for it to exit, and exiting requires m_lock. Holding
    //  m_lock while a task slot is destroyed would deadlock, so slot
    //  operations happen under m_task_lock with m_lock released.
    Mutex m_task_lock;

    int m_listen = -1;
    int m_client = -1;
    bool m_close_requested = false;

    //  A connection token, incremented (under m_lock) on every accept.
    //  send() compares it against the token the caller captured, so a
    //  write that outlived its connection cannot reach the client that
    //  replaced it. 0 means no connection is active.
    uint64_t m_connection_id = 0;

    //  One task per start() call. start() must never destroy the previous
    //  task's AsyncTask here: its destructor blocks until the task has
    //  finished, and a task from an earlier attempt may still be running
    //  (the caller's wait timed out while the pool was starved).
    //  close_and_wait() joins the whole list.
    std::vector<AsyncTask> m_accept_threads;
    AsyncTask m_client_thread;
};



}
#endif
