/*  Server Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_POSIX_H
#define PokemonAutomation_ServerSocket_POSIX_H

#include <string>
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
        close();
    }

    virtual void start(const std::string& address, uint16_t port) override;

    //  Closes the active connection and keeps listening. The connection
    //  thread stops and closes the socket itself; an in-flight "send()" is
    //  cancelled and returns promptly with the bytes written so far (the
    //  send loop re-checks the state every iteration). Safe to call from
    //  within a listener callback.
    virtual void close_connection() override;

    //  Stops the server. It cancels any in-flight "send()" the same way
    //  "close_connection()" does, and blocks until the worker threads have
    //  exited, so it must not be called from within a listener callback.
    virtual void close() noexcept override;

    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    ) override;


private:

    void accept_thread_loop(std::string address, uint16_t port);
    void connection_thread_loop();


private:
    ThreadPool& m_thread_pool;
    Mutex m_lock;
    ConditionVariable m_cv;
    bool m_starting = false;
    int m_closing = 0;

    //  Protects m_accept_thread and m_client_thread. Kept separate from
    //  m_lock: assigning to a task slot destroys the previous task's
    //  AsyncTask, and its destructor waits for that task to finish - which
    //  requires m_lock, so the two locks must never be held at once.
    Mutex m_task_lock;

    int m_listen = -1;
    int m_client = -1;
    bool m_close_requested = false;

    AsyncTask m_accept_thread;
    AsyncTask m_client_thread;
};



}
#endif
