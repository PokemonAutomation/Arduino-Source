/*  Server Socket (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32

#include <cstring>
#include <mutex>
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm>
#include <winsock2.h>
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "ServerSocket_WinSocket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace PokemonAutomation{

void ServerSocket_WinSocket::ensure_wsa_started(){
    static std::once_flag flag;
    std::call_once(flag, [](){
        WSADATA wsa_data;
        int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (ret != 0){
            try{
                std::cout << "WSAStartup() failed: WSA Error Code = " << ret << std::endl;
            }catch (...){}
        }
    });
}

void ServerSocket_WinSocket::start(const std::string& address, uint16_t port){
    //  Winsock has to be started before any of its functions can be
    //  used. Call it here, on the calling thread, so it is done before
    //  any socket is created (a client created right after "start()"
    //  must be able to use Winsock even before the listener thread has
    //  run).
    ensure_wsa_started();

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

uint64_t ServerSocket_WinSocket::connection_id(){
    std::lock_guard<Mutex> lg(m_lock);
    return m_state.load(std::memory_order_relaxed) == State::CONNECTED
        ? m_connection_id
        : 0;
}


void ServerSocket_WinSocket::close_connection(){
    SOCKET socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
            return;
        }
        m_close_requested = true;
        m_state.store(State::LISTENING, std::memory_order_release);
        socket = m_client;
    }
    if (socket != INVALID_SOCKET){
        //  Tells the connection thread to stop; it closes the socket
        //  itself. This is safe to call from within a listener callback.
        ::shutdown(socket, SD_BOTH);
    }
}

void ServerSocket_WinSocket::close_and_wait() noexcept{
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

    std::vector<AsyncTask> accept_threads;
    AsyncTask client_thread;
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

size_t ServerSocket_WinSocket::send(const void* data, size_t bytes, std::chrono::milliseconds timeout, uint64_t connection){
    SOCKET socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::CONNECTED
            || (connection != 0 && m_connection_id != connection)
        ){
            return 0;
        }
        socket = m_client;
    }
    if (socket == INVALID_SOCKET){
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
            //  The connection may have been replaced (or closed) since the
            //  last iteration: in that case this write must stop rather
            //  than reach the new client's socket.
            if (m_state.load(std::memory_order_relaxed) != State::CONNECTED
                || (connection != 0 && m_connection_id != connection)
            ){
                return sent;
            }
        }
        if (std::chrono::steady_clock::now() >= deadline){
            //  The deadline elapsed with bytes still unwritten; the
            //  connection is still fine, so just report the partial count.
            return sent;
        }

        //  "::send" takes an "int" length, so chunk the send the same
        //  way the WinSocket client does; sending a payload larger than
        //  2 GiB in one call would wrap the length negative.
        constexpr size_t BLOCK_SIZE = (size_t)1 << 30;
        const int current = (int)std::min<size_t>(bytes, BLOCK_SIZE);
        int current_sent = ::send(socket, ptr, current, 0);
        if (current_sent != SOCKET_ERROR){
            sent += (size_t)current_sent;
            ptr += current_sent;
            bytes -= (size_t)current_sent;
            continue;
        }

        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        //  WSAECONNRESET, WSAENOTCONN, or any other error: the
        //  connection is gone.
        return sent;
    }

    return sent;
}

void ServerSocket_WinSocket::report_listen_finished(uint64_t generation, const std::string& error){
    bool current;
    {
        std::lock_guard<Mutex> lg(m_lock);
        current = (m_generation == generation);
    }
    if (current){
        m_listeners.run_method(&Listener::on_listen_finished, error);
    }
}

void ServerSocket_WinSocket::accept_thread_loop(std::string address, uint16_t port, uint64_t generation){
    SOCKET listen_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET){
        report_listen_finished(
            generation,
            "socket() failed: WSA Error Code: " + std::to_string(WSAGetLastError())
        );
        return;
    }

    //  No SO_REUSEADDR: on Windows, that option lets another
    //  socket bind to the same address and port, so a second
    //  server could steal our port.
    u_long non_blocking = 1;
    ioctlsocket(listen_socket, FIONBIO, &non_blocking);

    sockaddr_in server;
    std::memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address.c_str());

    if (bind(listen_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR){
        int error = WSAGetLastError();
        ::closesocket(listen_socket);
        report_listen_finished(
            generation,
            "bind() failed: WSA Error Code: " + std::to_string(error)
        );
        return;
    }

    if (::listen(listen_socket, 16) == SOCKET_ERROR){
        int error = WSAGetLastError();
        ::closesocket(listen_socket);
        report_listen_finished(
            generation,
            "listen() failed: WSA Error Code: " + std::to_string(error)
        );
        return;
    }

    std::string start_error;
    bool superseded = false;
    {
        std::lock_guard<Mutex> lg(m_lock);
        superseded = m_generation != generation;
        if (superseded ||
            m_state.load(std::memory_order_relaxed) == State::DESTRUCTING
        ){
            //  A newer start() or close_and_wait() won the race.
            start_error = "The server was replaced or closed before it could start listening.";
        }else{
            m_listen = listen_socket;
            listen_socket = INVALID_SOCKET;
            m_state.store(State::LISTENING, std::memory_order_release);
        }
    }
    if (listen_socket != INVALID_SOCKET){
        ::closesocket(listen_socket);
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
        SOCKET listen;
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

        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(listen, &read_set);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        int ret = ::select(0, &read_set, nullptr, nullptr, &tv);
        if (ret == SOCKET_ERROR){
            if (WSAGetLastError() == WSAEINTR){
                continue;
            }
            break;
        }
        if (ret == 0){
            continue;
        }

        SOCKET client_socket = ::accept(listen, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET){
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS){
                continue;
            }
            break;
        }

        bool rejected = false;
        bool accepted = false;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING || state == State::NOT_RUNNING){
                ::closesocket(client_socket);
                break;
            }
            if (state == State::CONNECTED){
                //  A client is already connected: reject this one.
                ::closesocket(client_socket);
                rejected = true;
            }else{
                u_long client_non_blocking = 1;
                ioctlsocket(client_socket, FIONBIO, &client_non_blocking);
                m_client = client_socket;
                m_close_requested = false;
                m_connection_id++;
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
                ::closesocket(client_socket);
                m_client = INVALID_SOCKET;
                m_state.store(State::LISTENING, std::memory_order_release);
            }
        }

        if (rejected){
            m_listeners.run_method(&Listener::on_connect_rejected);
        }
    }

    SOCKET listen;
    {
        std::lock_guard<Mutex> lg(m_lock);
        listen = m_listen;
        m_listen = INVALID_SOCKET;
    }
    if (listen != INVALID_SOCKET){
        ::closesocket(listen);
    }
}

void ServerSocket_WinSocket::connection_thread_loop(){
    SOCKET socket;
    bool active;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_client;
        active = (m_state.load(std::memory_order_relaxed) == State::CONNECTED);
        if (!active && m_client == socket){
            m_client = INVALID_SOCKET;
        }
    }
    if (!active){
        //  The connection was closed before this thread started.
        ::closesocket(socket);
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
        int bytes = ::recv(socket, buffer, (int)BUFFER_SIZE, 0);

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
            m_listeners.run_method(&Listener::on_receive_data, buffer, (size_t)bytes);
            continue;
        }

        if (bytes == SOCKET_ERROR){
            int error_no = WSAGetLastError();
            if (error_no == WSAEWOULDBLOCK || error_no == WSAEINPROGRESS){
                //  No data yet.
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            if (error_no == WSAEINTR){
                continue;
            }
            error = "Receive error (WSA Error Code " + std::to_string(error_no) + ")";
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
            m_client = INVALID_SOCKET;
            if (m_state.load(std::memory_order_relaxed) == State::CONNECTED){
                m_state.store(State::LISTENING, std::memory_order_release);
                report_disconnect = !m_close_requested;
            }
        }
    }
    ::closesocket(socket);
    if (report_disconnect){
        m_listeners.run_method(&Listener::on_disconnect, error);
    }
}

}

#endif
