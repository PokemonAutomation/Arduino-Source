/*  Server Socket (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_WinSocket_H
#define PokemonAutomation_ServerSocket_WinSocket_H

#include <string>
#include <cstring>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
#include <winsock.h>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "AbstractServerSocket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace PokemonAutomation{


class ServerSocket_WinSocket final : public AbstractServerSocket{
public:
    ServerSocket_WinSocket(ThreadPool& thread_pool)
        : m_thread_pool(thread_pool)
    {}

    virtual ~ServerSocket_WinSocket(){
        close();
    }

    virtual void start(const std::string& address, uint16_t port) override{
        //  Winsock has to be started before any of its functions can be
        //  used. Call it here, on the calling thread, so it is done before
        //  any socket is created (a client created right after "start()"
        //  must be able to use Winsock even before the listener thread has
        //  run).
        ensure_wsa_started();

        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING
                || m_starting){
                return;
            }
            m_starting = true;
        }

        //  Use the plain dispatch() rather than dispatch_now_blocking():
        //  start() may be called from a thread that is itself running on
        //  this pool, and blocking here until a pool thread is free could
        //  deadlock a capped pool. The returned AsyncTask still lets
        //  close() join the listener thread.
        try{
            AsyncTask task = m_thread_pool.dispatch([=, this]{
                accept_thread_loop(address, port);
            });
            {
                //  m_task_lock, not m_lock: close() only reads the slot
                //  after it has waited for m_starting to clear, so this
                //  assignment is the only writer besides close().
                std::lock_guard<Mutex> lg(m_task_lock);
                m_accept_thread = std::move(task);
            }
        }catch (...){
            {
                std::lock_guard<Mutex> lg(m_lock);
                m_starting = false;
                m_cv.notify_all();
            }
            m_listeners.run_method(&Listener::on_listen_finished, "Failed to start the listener thread.");
            throw;
        }

        {
            std::lock_guard<Mutex> lg(m_lock);
            m_starting = false;
            m_cv.notify_all();
        }
    }

    virtual void close_connection() override{
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

    virtual void close() noexcept override{
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
            //  close() is already draining the threads (m_closing > 1).
            //  In the second case, wait for it to finish.
            std::unique_lock<Mutex> lg(m_lock);
            m_cv.wait(lg, [&]{
                return m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING;
            });
            return;
        }

        AsyncTask accept_thread;
        AsyncTask client_thread;
        {
            //  m_task_lock, not m_lock: moving the tasks out destroys the
            //  old AsyncTask objects, and their destructors wait for the
            //  tasks to finish - which they can only do if they can acquire
            //  m_lock.
            std::lock_guard<Mutex> lg(m_task_lock);
            accept_thread = std::move(m_accept_thread);
            m_accept_thread = AsyncTask();
            client_thread = std::move(m_client_thread);
            m_client_thread = AsyncTask();
        }

        //  The threads notice the new state within a few milliseconds and
        //  close their own sockets.
        accept_thread.wait_and_ignore_exceptions();
        client_thread.wait_and_ignore_exceptions();

        {
            std::lock_guard<Mutex> lg(m_lock);
            m_state.store(State::NOT_RUNNING, std::memory_order_release);
            m_close_requested = false;
            m_closing = 0;
            m_cv.notify_all();
        }
    }

    virtual size_t send(const void* data, size_t bytes) override{
        SOCKET socket;
        {
            std::lock_guard<Mutex> lg(m_lock);
            socket =
                (m_state.load(std::memory_order_relaxed) == State::CONNECTED) ? m_client : INVALID_SOCKET;
        }
        if (socket == INVALID_SOCKET){
            return 0;
        }

        const char* ptr = (const char*)data;
        size_t sent = 0;

        while (bytes > 0){
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
                    return sent;
                }
            }

            int current_sent = ::send(socket, ptr, (int)bytes, 0);
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


private:
    //  Winsock has to be started before any of its functions can be used, and
    //  nothing else in the program calls WSAStartup, so call it exactly once
    //  before the first socket is created. We deliberately never call
    //  WSACleanup: it is a process-wide, reference-counted operation, and
    //  other parts of the process (Qt) may be using Winsock as well.
    static void ensure_wsa_started(){
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

    void accept_thread_loop(std::string address, uint16_t port){
        SOCKET listen_socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_socket == INVALID_SOCKET){
            m_listeners.run_method(
                &Listener::on_listen_finished,
                "socket() failed: WSA Error Code: " + std::to_string(WSAGetLastError())
            );
            return;
        }

        int yes = 1;
        setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

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
            m_listeners.run_method(
                &Listener::on_listen_finished,
                "bind() failed: WSA Error Code: " + std::to_string(error)
            );
            return;
        }

        if (::listen(listen_socket, 16) == SOCKET_ERROR){
            int error = WSAGetLastError();
            ::closesocket(listen_socket);
            m_listeners.run_method(
                &Listener::on_listen_finished,
                "listen() failed: WSA Error Code: " + std::to_string(error)
            );
            return;
        }

        std::string start_error;
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) == State::DESTRUCTING){
                //  close() won the race.
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
                if (state == State::DESTRUCTING || state == State::NOT_RUNNING){
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
                    u_long non_blocking = 1;
                    ioctlsocket(client_socket, FIONBIO, &non_blocking);
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

    void connection_thread_loop(){
        m_listeners.run_method(&Listener::on_thread_start);

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
            int bytes;
            int error_no = 0;
            bool no_data;
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
                    break;
                }
                bytes = ::recv(socket, buffer, (int)BUFFER_SIZE, 0);
                no_data = (bytes == SOCKET_ERROR);
                if (no_data){
                    error_no = WSAGetLastError();
                    no_data = (error_no == WSAEWOULDBLOCK || error_no == WSAEINPROGRESS);
                }
            }

            if (bytes > 0){
                m_listeners.run_method(&Listener::on_receive_data, buffer, (size_t)bytes);
                continue;
            }

            if (no_data){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            if (bytes == 0){
                error = "Connection closed by client.";
            }else{
                switch (error_no){
                case WSAEINTR:
                    continue;
                default:
                    error = "Receive error (WSA Error Code " + std::to_string(error_no) + ")";
                    break;
                }
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
