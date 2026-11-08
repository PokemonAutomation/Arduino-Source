/*  Server Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#if !_WIN32

#include <cerrno>
#include <csignal>
#include <cstring>
#include <chrono>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "ServerSocket_POSIX.h"

namespace PokemonAutomation{

ServerSocket_POSIX::ServerSocket_POSIX(ThreadPool& thread_pool)
    : m_thread_pool(thread_pool)
{
    // Ignore SIGPIPE. Handle errors via errno instead
    signal(SIGPIPE, SIG_IGN);
}

void ServerSocket_POSIX::start(const std::string& address, uint16_t port){
    uint64_t generation = 0;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_starting ||
            m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING
        ){
            return;
        }
        m_starting = true;
        m_generation++;
        generation = m_generation;
    }

    //  Use the plain dispatch() rather than dispatch_now_blocking():
    //  start() may be called from a thread that is itself running on
    //  this pool, and blocking here until a pool thread is free could
    //  deadlock a capped pool.
    try{
        AsyncTask task = m_thread_pool.dispatch([=, this]{
            accept_thread_loop(address, port, generation);
        });
        {
            //  Append, never replace: replacing the slot would destroy
            //  the previous task's AsyncTask, and its destructor blocks
            //  until that task has finished. A task from an earlier
            //  start() may still be running when this start() runs (the
            //  caller's wait timed out while the pool was starved). It
            //  notices its stale generation when it gets scheduled and
            //  exits without listening, so it only has to outlive this
            //  call; close_and_wait() joins the whole list.
            std::lock_guard<Mutex> lg(m_task_lock);
            m_accept_threads.push_back(std::move(task));
        }
    }catch (...){
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_starting = false;
            m_cv.notify_all();
        }
        m_listeners.run_method(
            &Listener::on_listen_finished,
            "Failed to start the listener thread."
        );
        throw;
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        m_starting = false;
        m_cv.notify_all();
    }
}

void ServerSocket_POSIX::close_connection(){
    int socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
            return;
        }
        m_close_requested = true;
        m_state.store(State::LISTENING, std::memory_order_release);
        socket = m_client;
    }
    if (socket != -1){
        //  Tells the connection thread to stop; it closes the socket
        //  itself. This is safe to call from within a listener callback.
        ::shutdown(socket, SHUT_RDWR);
    }
}

void ServerSocket_POSIX::close_and_wait() noexcept{
    {
        std::unique_lock<Mutex> lg(m_lock);
        //  Wait for an in-progress start() to publish the listener task
        //  before we take the task slots.
        m_cv.wait(lg, [&]{ return !m_starting; });
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
            m_close_requested = true;
            m_state.store(State::DESTRUCTING, std::memory_order_release);
            m_closing++;
        }
    }

    bool drain = false;
    {
        std::lock_guard<Mutex> lg(m_lock);
        drain = (m_closing == 1);
    }
    if (!drain){
        //  Either nothing was running (m_closing == 0), or another
        //  close_and_wait() is already draining the threads (m_closing > 1).
        //  In the second case, wait for it to finish.
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [&]{
            return m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING;
        });
        return;
    }

    AsyncTask client_thread;
    std::vector<AsyncTask> accept_threads;
    {
        //  m_task_lock, not m_lock: moving the tasks out destroys the
        //  old AsyncTask objects, and their destructors wait for the
        //  tasks to finish - which they can only do if they can acquire
        //  m_lock.
        std::lock_guard<Mutex> lg(m_task_lock);
        accept_threads.swap(m_accept_threads);
        client_thread = std::move(m_client_thread);
        m_client_thread = AsyncTask();
    }

    //  The threads notice the new state within a few milliseconds and
    //  close their own sockets.
    for (auto& task : accept_threads){
        task.wait_and_ignore_exceptions();
    }
    client_thread.wait_and_ignore_exceptions();

    {
        std::lock_guard<Mutex> lg(m_lock);
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        m_close_requested = false;
        m_closing = 0;
        m_cv.notify_all();
    }
}

size_t ServerSocket_POSIX::send(const void* data, size_t bytes, std::chrono::milliseconds timeout){
    int socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_state.load(std::memory_order_relaxed) == State::CONNECTED
            ? m_client
            : -1;
    }
    if (socket == -1){
        return 0;
    }

    const char* ptr = (const char*)data;
    size_t sent = 0;

    //  "time_point::max()" (the infinite default) is never reached; using
    //  it also avoids overflowing "now() + timeout".
    const std::chrono::steady_clock::time_point deadline =
        (timeout == std::chrono::milliseconds::max())
            ? std::chrono::steady_clock::time_point::max()
            : std::chrono::steady_clock::now() + timeout;

    while (bytes > 0){
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
                return sent;
            }
        }
        if (std::chrono::steady_clock::now() >= deadline){
            //  The deadline elapsed with bytes still unwritten; the
            //  connection is still fine, so just report the partial count.
            return sent;
        }

        constexpr size_t BLOCK_SIZE = 1 << 30;
        size_t current = std::min(BLOCK_SIZE, bytes);
        ssize_t current_sent = ::send(socket, ptr, current, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (current_sent != -1){
            sent += (size_t)current_sent;
            ptr += current_sent;
            bytes -= (size_t)current_sent;
            continue;
        }

        int error = errno;
        switch (error){
        case EAGAIN:
#if EAGAIN != EWOULDBLOCK
        case EWOULDBLOCK:
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        default:
            //  EPIPE, ECONNRESET, or any other error: the connection is
            //  gone.
            return sent;
        }
    }

    return sent;
}

void ServerSocket_POSIX::report_listen_finished(uint64_t generation, const std::string& error){
    bool current;
    {
        std::lock_guard<Mutex> lg(m_lock);
        current = (m_generation == generation);
    }
    if (current){
        m_listeners.run_method(&Listener::on_listen_finished, error);
    }
}

void ServerSocket_POSIX::accept_thread_loop(std::string address, uint16_t port, uint64_t generation){
    int listen_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1){
        m_listeners.run_method(
            &Listener::on_listen_finished,
            "Failed to create the socket: " + std::string(strerror(errno))
        );
        return;
    }

    int yes = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in server;
    std::memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address.c_str());

    if (bind(listen_socket, (struct sockaddr*)&server, sizeof(server)) == -1){
        int error = errno;
        ::close(listen_socket);
        report_listen_finished(
            generation,
            "Failed to bind the socket: " + std::string(strerror(error))
        );
        return;
    }

    if (::listen(listen_socket, 16) == -1){
        int error = errno;
        ::close(listen_socket);
        report_listen_finished(
            generation,
            "Failed to listen on the socket: " + std::string(strerror(error))
        );
        return;
    }

    std::string start_error;
    bool superseded = false;
    {
        std::lock_guard<Mutex> lg(m_lock);
        superseded = (m_generation != generation);
        if (superseded ||
            m_state.load(std::memory_order_relaxed) == State::DESTRUCTING
        ){
            //  A newer start() or close_and_wait() won the race.
            start_error = "The server was replaced or closed before it could start listening.";
        }else{
            m_listen = listen_socket;
            listen_socket = -1;
            m_state.store(State::LISTENING, std::memory_order_release);
        }
    }
    if (listen_socket != -1){
        ::close(listen_socket);
    }

    if (superseded){
        //  The attempt that started this task gave up on it while it was
        //  still starting up (its wait timed out). A newer attempt owns
        //  the listeners now; reporting this result would only confuse
        //  that attempt, so exit without touching them.
        return;
    }

    m_listeners.run_method(&Listener::on_listen_finished, start_error);
    if (!start_error.empty()){
        return;
    }

    //  Accept loop. Runs for the whole lifetime of the server: it keeps
    //  polling while a connection is active, and rejects new clients in
    //  the meantime.
    while (true){
        int listen;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING ||
                state == State::NOT_RUNNING ||
                m_generation != generation
            ){
                break;
            }
            listen = m_listen;
        }

        pollfd pf;
        pf.fd = listen;
        pf.events = POLLIN;
        pf.revents = 0;
        int ret = ::poll(&pf, 1, 10);
        if (ret == -1){
            if (errno == EINTR){
                continue;
            }
            break;
        }
        if (ret == 0){
            continue;
        }

        int client_socket = ::accept(listen, nullptr, nullptr);
        if (client_socket == -1){
            continue;
        }

        bool rejected = false;
        bool accepted = false;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING || state == State::NOT_RUNNING){
                ::close(client_socket);
                break;
            }
            if (state == State::CONNECTED){
                //  A client is already connected: reject this one.
                ::close(client_socket);
                rejected = true;
            }else{
                fcntl(client_socket, F_SETFL, O_NONBLOCK);
                m_client = client_socket;
                m_close_requested = false;
                m_state.store(State::CONNECTED, std::memory_order_release);
                accepted = true;
            }
        }

        if (accepted){
            //  Start the connection thread without holding m_lock.
            //  The assignment to m_client_thread destroys the previous
            //  connection task's AsyncTask, whose destructor waits for
            //  that task to finish - and that task may be blocked on
            //  m_lock.
            try{
                AsyncTask task = m_thread_pool.dispatch([this]{
                    connection_thread_loop();
                });
                std::lock_guard<Mutex> lg(m_task_lock);
                m_client_thread = std::move(task);
            }catch (...){
                //  The connection thread could not be started. Drop
                //  this connection and keep listening.
                std::lock_guard<Mutex> lg(m_lock);
                ::close(client_socket);
                m_client = -1;
                m_state.store(State::LISTENING, std::memory_order_release);
            }
        }

        if (rejected){
            m_listeners.run_method(&Listener::on_connect_rejected);
        }
    }

    int listen;
    {
        std::lock_guard<Mutex> lg(m_lock);
        listen = m_listen;
        m_listen = -1;
    }
    if (listen != -1){
        ::close(listen);
    }
}

void ServerSocket_POSIX::connection_thread_loop(){
    int socket;
    bool active;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_client;
        active = (m_state.load(std::memory_order_relaxed) == State::CONNECTED);
        if (!active && m_client == socket){
            m_client = -1;
        }
    }
    if (!active){
        //  The connection was closed before this thread started.
        ::close(socket);
        return;
    }

    m_listeners.run_method(&Listener::on_connect);

    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::string error;

    while (true){
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
                break;
            }
        }

        //  The socket is non-blocking, so this returns immediately. It is
        //  deliberately called without holding m_lock: the state check
        //  above and the result handling below are the only parts that
        //  need the lock, and no socket call should stretch it.
        ssize_t bytes = ::recv(socket, buffer, BUFFER_SIZE, 0);

        if (bytes > 0){
            //  A new connection may have taken over while we were
            //  reading: only dispatch the data if we still are the
            //  active connection.
            bool still_active;
            {
                std::lock_guard<Mutex> lg(m_lock);
                still_active = (m_client == socket);
            }
            if (!still_active){
                break;
            }
            m_listeners.run_method(&Listener::on_receive_data, buffer, bytes);
            continue;
        }

        if (bytes == -1){
            int error_no = errno;
            if (error_no == EAGAIN){
                //  No data yet.
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
#if EAGAIN != EWOULDBLOCK
            if (error_no == EWOULDBLOCK){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
#endif
            if (error_no == EINTR){
                continue;
            }
            error = "Receive error (errno " + std::to_string(error_no) + "): " + std::string(strerror(error_no));
        }else if (bytes == 0){
            error = "Connection closed by client.";
        }
        break;
    }

    bool report_disconnect;
    {
        std::lock_guard<Mutex> lg(m_lock);
        report_disconnect = false;
        if (m_client == socket){
            //  Still the active connection (or it was closed before a
            //  new one took over): we own the socket and the state.
            m_client = -1;
            if (m_state.load(std::memory_order_relaxed) == State::CONNECTED){
                m_state.store(State::LISTENING, std::memory_order_release);
                report_disconnect = !m_close_requested;
            }
        }
    }
    ::close(socket);
    if (report_disconnect){
        m_listeners.run_method(&Listener::on_disconnect, error);
    }
}

}

#endif
