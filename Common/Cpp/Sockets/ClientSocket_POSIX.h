/*  Client Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_POSIX_H
#define PokemonAutomation_ClientSocket_POSIX_H

#include <atomic>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "AbstractClientSocket.h"

namespace PokemonAutomation{


class ThreadPool;



class ClientSocket_POSIX final : public AbstractClientSocket{
public:
    ClientSocket_POSIX(ThreadPool& thread_pool);

    virtual ~ClientSocket_POSIX(){
        close();
    }

    //  Stops the receive thread, closes the socket, and returns the socket to
    //  State::NOT_RUNNING so "connect()" can be called again. This is safe to
    //  call multiple times.
    //
    //  This blocks until the receive thread has exited, so it must not be
    //  called from within a listener callback (which runs on that thread).
    virtual void close() noexcept override;

    virtual void connect(const std::string& address, uint16_t port) override;

    virtual size_t send(const void* data, size_t bytes) override;


private:
    //  Closes the underlying socket, if any. Requires "m_lock" to be held.
    void close_socket_locked();

    //  A connection ended on its own (the connect attempt failed, the server
    //  closed the connection, or a socket error occurred). This returns the
    //  socket to State::NOT_RUNNING so "connect()" can be called again, and
    //  fires the appropriate callback outside of "m_lock".
    //  Requires "lg" to be locked. It unlocks it.
    void connection_lost_locked(
        std::unique_lock<Mutex>& lg,
        const std::string& error_message,
        bool was_connected
    );

    void thread_loop(std::string address, uint16_t port);
    void thread_loop_internal(std::string address, uint16_t port);


private:
    ThreadPool& m_thread_pool;
    std::atomic<int> m_socket{-1};

    std::string m_error;

    mutable Mutex m_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;
};



}
#endif
