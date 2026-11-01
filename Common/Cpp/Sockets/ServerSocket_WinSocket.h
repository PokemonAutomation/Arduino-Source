/*  Server Socket (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_WinSocket_H
#define PokemonAutomation_ServerSocket_WinSocket_H

#include <string>
#include <atomic>
#include <winsock2.h>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "AbstractServerSocket.h"

namespace PokemonAutomation{


class ThreadPool;



class ServerSocket_WinSocket final : public AbstractServerSocket{
public:
    ServerSocket_WinSocket(ThreadPool& thread_pool)
        : m_thread_pool(thread_pool)
    {}

    virtual ~ServerSocket_WinSocket(){
        close();
    }

    virtual void start(const std::string& address, uint16_t port) override;

    virtual void close_connection() override;

    virtual void close() noexcept override;

    virtual size_t send(const void* data, size_t bytes) override;


private:
    //  Winsock has to be started before any of its functions can be used, and
    //  nothing else in the program calls WSAStartup, so call it exactly once
    //  before the first socket is created. We deliberately never call
    //  WSACleanup: it is a process-wide, reference-counted operation, and
    //  other parts of the process (Qt) may be using Winsock as well.
    static void ensure_wsa_started();

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

    SOCKET m_listen = INVALID_SOCKET;
    SOCKET m_client = INVALID_SOCKET;
    bool m_close_requested = false;

    AsyncTask m_accept_thread;
    AsyncTask m_client_thread;
};



}
#endif
