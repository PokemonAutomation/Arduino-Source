/*  Server Socket 2 (Windows)
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
#include "ServerSocket2_WinSocket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace PokemonAutomation{

size_t ServerSocket2Connection_WinSocket::send(
    const void* data,
    size_t bytes,
    std::chrono::milliseconds timeout
){
    if (!m_open.load(std::memory_order_relaxed)){
        return 0;
    }
    std::lock_guard<Mutex> lg(m_send_lock);
    if (!m_open.load(std::memory_order_relaxed)){
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
        //  The connection may have closed (by either side) since the last
        //  iteration: this write must stop rather than reach a dead
        //  socket.
        if (!m_open.load(std::memory_order_relaxed)){
            return sent;
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
        int current_sent = ::send(m_client, ptr, current, 0);
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
        //  connection is gone; report the bytes written so far.
        return sent;
    }

    return sent;
}

void ServerSocket2Connection_WinSocket::close(){
    if (m_close_requested.exchange(true)){
        return;
    }
    m_open.store(false, std::memory_order_release);
    //  Tells the connection thread to stop; it closes the socket
    //  itself. Safe to call from within a listener callback. (If the
    //  socket is already gone, ::shutdown just fails harmlessly.)
    ::shutdown(m_client, SD_BOTH);
}


void ServerSocket2_WinSocket::ensure_wsa_started(){
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

ServerSocket2_WinSocket::ServerSocket2_WinSocket(ThreadPool& thread_pool)
    : m_thread_pool(thread_pool)
{
}

void ServerSocket2_WinSocket::start(const std::string& address, uint16_t port){
    //  Winsock has to be started before any of its functions can be
    //  used. Call it here, on the calling thread, so it is done before
    //  any socket is created (a client created right after "start()"
    //  must be able to use Winsock even before the listener thread has
    //  run).
    ensure_wsa_started();

    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_starting ||
            m_task_alive ||
            m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING
        ){
            return;
        }
        m_starting = true;
        m_task_alive = true;
    }

    //  Use the plain dispatch() rather than dispatch_now_blocking():
    //  start() may be called from a thread that is itself running on
    //  this pool, and blocking here until a pool thread is free could
    //  deadlock a capped pool.
    try{
        AsyncTask task = m_thread_pool.dispatch([=, this]{
            accept_thread_loop(address, port);
        });
        {
            //  Append, never replace: replacing the slot would destroy
            //  the previous task's AsyncTask, and its destructor blocks
            //  until that task has finished. The previous tasks here are
            //  all failed attempts (start() is a no-op while one is
            //  alive), so theirs return immediately; the live one is
            //  joined by close_and_wait().
            std::lock_guard<Mutex> lg(m_task_lock);
            m_accept_threads.push_back(std::move(task));
        }
    }catch (...){
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_starting = false;
            m_task_alive = false;
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

void ServerSocket2_WinSocket::close_and_wait() noexcept{
    {
        std::unique_lock<Mutex> lg(m_lock);
        //  Wait for an in-progress start() to publish the listener task
        //  before we take the task slots.
        m_cv.wait(lg, [&]{ return !m_starting; });
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
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
        //  close_and_wait() is already draining the threads (m_closing >
        //  1). In the second case, wait for it to finish.
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [&]{
            return m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING;
        });
        return;
    }

    //  Close the active connection first: it makes its connection thread
    //  stop, so the join below cannot wait forever. The connection object
    //  itself outlives the server (the listeners keep their references);
    //  it only becomes inert.
    ConnectionPtr active;
    {
        std::lock_guard<Mutex> lg(m_lock);
        active = m_active;
    }
    if (active){
        active->close();
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
        m_active = ConnectionPtr();
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        m_task_alive = false;
        m_closing = 0;
        m_cv.notify_all();
    }
}

void ServerSocket2_WinSocket::listen_failed(const std::string& error){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_task_alive = false;
    }
    m_listeners.run_method(&Listener::on_listen_finished, error);
}

void ServerSocket2_WinSocket::accept_thread_loop(std::string address, uint16_t port){
    SOCKET listen_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET){
        listen_failed("socket() failed: WSA Error Code: " + std::to_string(WSAGetLastError()));
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
        listen_failed("bind() failed: WSA Error Code: " + std::to_string(error));
        return;
    }

    if (::listen(listen_socket, 16) == SOCKET_ERROR){
        int error = WSAGetLastError();
        ::closesocket(listen_socket);
        listen_failed("listen() failed: WSA Error Code: " + std::to_string(error));
        return;
    }

    std::string start_error;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) == State::DESTRUCTING){
            //  close_and_wait() won the race: the task was still
            //  starting up when the server was destroyed.
            start_error = "The server was closed before it could start listening.";
        }else{
            m_listen = listen_socket;
            listen_socket = INVALID_SOCKET;
            m_state.store(State::LISTENING, std::memory_order_release);
        }
    }
    if (listen_socket != INVALID_SOCKET){
        ::closesocket(listen_socket);
    }

    if (!start_error.empty()){
        listen_failed(start_error);
        return;
    }

    m_listeners.run_method(&Listener::on_listen_finished, "");

    //  Accept loop. Runs for the whole lifetime of the server: it keeps
    //  polling while a connection is active, and rejects new clients in
    //  the meantime.
    while (true){
        SOCKET listen;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING ||
                state == State::NOT_RUNNING
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
        std::shared_ptr<ServerSocket2Connection_WinSocket> conn;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING || state == State::NOT_RUNNING){
                ::closesocket(client_socket);
                break;
            }
            if (state == State::CONNECTED){
                //  A connection is already active: reject this client.
                ::closesocket(client_socket);
                rejected = true;
            }else{
                u_long client_non_blocking = 1;
                ioctlsocket(client_socket, FIONBIO, &client_non_blocking);
                //  Plain "new" + wrap, not make_shared: the constructor
                //  is private, and this call site is in the friend.
                conn = std::shared_ptr<ServerSocket2Connection_WinSocket>(
                    new ServerSocket2Connection_WinSocket(*this, client_socket)
                );
                m_active = conn;
                m_state.store(State::CONNECTED, std::memory_order_release);
                accepted = true;
            }
        }

        if (accepted){
            //  Start the connection thread without holding m_lock.
            //  The assignment to m_client_thread destroys the previous
            //  connection task's AsyncTask, whose destructor waits for
            //  that task to finish - and that task may be blocked on
            //  m_lock. The thread captures the connection's shared
            //  pointer, so the connection outlives this block (and the
            //  server's reference keeps it alive while the thread runs).
            try{
                AsyncTask task = m_thread_pool.dispatch([conn]{
                    conn->run();
                });
                std::lock_guard<Mutex> lg(m_task_lock);
                m_client_thread = std::move(task);
            }catch (...){
                //  The connection thread could not be started. Drop
                //  this connection and keep listening.
                std::lock_guard<Mutex> lg(m_lock);
                ::closesocket(client_socket);
                m_active = ConnectionPtr();
                m_state.store(State::LISTENING, std::memory_order_release);
                conn->mark_closed();
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

void ServerSocket2_WinSocket::report_connect(ConnectionPtr conn){
    m_listeners.run_method(&Listener::on_connect, conn);
}

void ServerSocket2_WinSocket::report_receive(ConnectionPtr conn, const void* data, size_t bytes){
    m_listeners.run_method(&Listener::on_receive_data, conn, data, bytes);
}

void ServerSocket2_WinSocket::report_disconnect(ConnectionPtr conn, const std::string& error){
    m_listeners.run_method(&Listener::on_disconnect, conn, error);
}

void ServerSocket2_WinSocket::release_connection(ConnectionPtr conn){
    std::lock_guard<Mutex> lg(m_lock);
    if (m_active.get() != conn.get()){
        return;
    }
    m_active = ConnectionPtr();
    if (m_state.load(std::memory_order_relaxed) == State::CONNECTED){
        m_state.store(State::LISTENING, std::memory_order_release);
    }
}

void ServerSocket2Connection_WinSocket::run(){
    std::shared_ptr<ServerSocket2Connection_WinSocket> self = shared_from_this();
    SOCKET socket = m_client;

    if (!m_open.load(std::memory_order_relaxed)){
        //  Closed before this thread started (teardown, or the
        //  application via active_connection()): hand the slot back and
        //  drop the socket; there is nothing to report.
        m_server->release_connection(self);
        ::closesocket(socket);
        return;
    }

    m_server->report_connect(self);

    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::string error;

    while (m_open.load(std::memory_order_relaxed)){
        //  The socket is non-blocking, so this returns immediately. It is
        //  deliberately called without holding any lock: the liveness
        //  check above and the result handling below are the only parts
        //  that need one, and no socket call should stretch it.
        int bytes = ::recv(socket, buffer, (int)BUFFER_SIZE, 0);

        if (bytes > 0){
            //  The application may have closed the connection while we
            //  were reading: only dispatch the data if it is still open.
            if (m_open.load(std::memory_order_relaxed)){
                m_server->report_receive(self, buffer, (size_t)bytes);
            }
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
            //  WSAECONNRESET, or any other error: the connection is
            //  gone. (An application-side close() also lands here, via
            //  ::shutdown: it makes recv fail or return 0, and the
            //  close_requested() check below suppresses the report.)
            error = "Receive error (WSA Error Code " + std::to_string(error_no) + ")";
        }else{
            //  The peer closed the connection.
            error = "Connection closed by client.";
        }
        break;
    }

    //  Hand the slot (and the CONNECTED state) back to the server, then
    //  make the connection inert. Either way, the object outlives the
    //  server.
    m_server->release_connection(self);
    const bool app_close = m_close_requested.load(std::memory_order_relaxed);
    m_open.store(false, std::memory_order_release);
    ::closesocket(socket);
    if (!app_close){
        m_server->report_disconnect(self, error);
    }
}

}

#endif
