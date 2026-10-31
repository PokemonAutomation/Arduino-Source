/*  Client Socket (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_WinSocket_H
#define PokemonAutomation_ClientSocket_WinSocket_H

#include <atomic>
#include <iostream>
#include <winsock.h>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "AbstractClientSocket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace PokemonAutomation{



class ClientSocket_WinSocket final : public AbstractClientSocket{
public:
    ClientSocket_WinSocket(ThreadPool& thread_pool)
        : m_thread_pool(thread_pool)
    {}

    virtual ~ClientSocket_WinSocket(){
        close();
    }

    //  Stops the receive thread, closes the socket, and returns the socket to
    //  State::NOT_RUNNING so "connect()" can be called again. This is safe to
    //  call multiple times.
    //
    //  This blocks until the receive thread has exited, so it must not be
    //  called from within a listener callback (which runs on that thread).
    virtual void close() noexcept override{
        AsyncTask thread;
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING && !m_thread){
                //  Never connected, or already fully closed.
                return;
            }
            m_state.store(State::DESTRUCTING, std::memory_order_release);
            m_cv.notify_all();
            thread = std::move(m_thread);
        }

        thread.wait_and_ignore_exceptions();

        {
            std::lock_guard<Mutex> lg(m_lock);
            close_socket_locked();
            m_state.store(State::NOT_RUNNING, std::memory_order_release);
            m_cv.notify_all();
        }
    }

    virtual void connect(const std::string& address, uint16_t port) override{
        std::string error;
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
                return;
            }

            //  A socket whose previous connection has ended cannot be
            //  reconnected, so always start from a fresh socket.
            close_socket_locked();

            SOCKET socket = ::socket(AF_INET, SOCK_STREAM, 0);
            if (socket == INVALID_SOCKET){
                error = "socket() failed: WSA Error Code: " + std::to_string(WSAGetLastError());
            }else{
                u_long non_blocking = 1;
                if (ioctlsocket(socket, FIONBIO, &non_blocking) != 0){
                    error = "ioctlsocket() failed: WSA Error Code: " + std::to_string(WSAGetLastError());
                    ::closesocket(socket);
                }else{
                    m_socket.store(socket, std::memory_order_release);
                }
            }
            if (!error.empty()){
                m_error = error;
                return;
            }

            m_state.store(State::CONNECTING, std::memory_order_relaxed);
        }
        if (!error.empty()){
            m_listeners.run_method(&Listener::on_connect_finished, error);
            return;
        }

        try{
            m_thread = m_thread_pool.dispatch_now_blocking([=, this]{
                thread_loop(address, port);
            });
        }catch (...){
            {
                std::lock_guard<Mutex> lg(m_lock);
                m_error = "Failed to start the receive thread.";
                close_socket_locked();
                m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
            }
            m_listeners.run_method(&Listener::on_connect_finished, m_error);
            throw;
        }
    }


    virtual size_t send(const void* data, size_t bytes) override{
        const char* ptr = (const char*)data;
        size_t sent = 0;

        while (bytes > 0 && state() == State::CONNECTED){
            SOCKET socket = m_socket.load(std::memory_order_acquire);
            if (socket == INVALID_SOCKET){
                return sent;
            }

            constexpr size_t BLOCK_SIZE = (size_t)1 << 30;
            int current = (int)std::min<size_t>(bytes, BLOCK_SIZE);
            int current_sent = ::send(socket, ptr, current, 0);
            if (current_sent != SOCKET_ERROR){
                sent += current_sent;
                if (current_sent != current){
                    return sent;
                }
                ptr += current_sent;
                bytes -= current_sent;
                continue;
            }

            int error = WSAGetLastError();
//            cout << "error = " << error << endl;

            std::unique_lock<Mutex> lg(m_lock);
            if (state() == State::DESTRUCTING){
                break;
            }

            switch (error){
            case WSAEWOULDBLOCK:
                break;
            default:
                m_error = "WSA Error Code: " + std::to_string(error);
                return sent;
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }
        return sent;
    }


private:
    //  Closes the underlying socket, if any. Requires "m_lock" to be held.
    void close_socket_locked(){
        SOCKET socket = m_socket.exchange(INVALID_SOCKET, std::memory_order_acquire);
        if (socket == INVALID_SOCKET){
            return;
        }
        if (::closesocket(socket) != 0){
            try{
                std::cout << "Failed closesocket(): WSA Error Code = " + std::to_string(WSAGetLastError()) << std::endl;
            }catch (...){}
        }
    }

    //  A connection ended on its own (the connect attempt failed, the server
    //  closed the connection, or a socket error occurred). This returns the
    //  socket to State::NOT_RUNNING so "connect()" can be called again, and
    //  fires the appropriate callback outside of "m_lock".
    //  Requires "lg" to be locked. It unlocks it.
    void connection_lost_locked(
        std::unique_lock<Mutex>& lg,
        const std::string& error_message,
        bool was_connected
    ){
        m_error = error_message;
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        close_socket_locked();
        lg.unlock();

        if (was_connected){
            m_listeners.run_method(&Listener::on_disconnect, error_message);
        }else{
            m_listeners.run_method(&Listener::on_connect_finished, error_message);
        }
    }

    void thread_loop(const std::string& address, uint16_t port){
        try{
            thread_loop_internal(address, port);
        }catch (...){
            try{
                std::cout << "ClientSocket_WinSocket(): An exception was thrown from the receive thread." << std::endl;
            }catch (...){}
        }
    }

    void thread_loop_internal(const std::string& address, uint16_t port){
        m_listeners.run_method(&Listener::on_thread_start);

        {
            std::unique_lock<Mutex> lg(m_lock);

            sockaddr_in server;
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            server.sin_addr.s_addr = inet_addr(address.c_str());

            //  Connect
            while (true){
                State state = m_state.load(std::memory_order_relaxed);
                if (state == State::DESTRUCTING){
                    return;
                }

                SOCKET socket = m_socket.load(std::memory_order_acquire);
                if (socket == INVALID_SOCKET){
                    return;
                }

                if (::connect(socket, (struct sockaddr*)&server, sizeof(server)) != SOCKET_ERROR){
                    break;
                }

                int error = WSAGetLastError();
//                cout << "error = " << error << endl;

                switch (error){
                case WSAEISCONN:
                    goto Connected;
                case WSAEWOULDBLOCK:
                    break;
                case WSAETIMEDOUT:
                    connection_lost_locked(lg, "Connection timed out.", false);
                    return;
                default:
                    connection_lost_locked(lg, "WSA Error Code: " + std::to_string(error), false);
                    return;
                }

                m_cv.wait_for(lg, std::chrono::milliseconds(10));

            }

Connected:
            m_state.store(State::CONNECTED, std::memory_order_relaxed);
        }

        m_listeners.run_method(&Listener::on_connect_finished, "");


        constexpr size_t BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE];

        while (true){
            int bytes;
            int error = 0;
            {
                State state = m_state.load(std::memory_order_relaxed);
                if (state == State::DESTRUCTING){
                    return;
                }

                SOCKET socket = m_socket.load(std::memory_order_acquire);
                if (socket == INVALID_SOCKET){
                    return;
                }

                bytes = ::recv(socket, buffer, BUFFER_SIZE, 0);
                if (bytes == SOCKET_ERROR){
                    error = WSAGetLastError();
                }
            }

            if (bytes > 0){
                m_listeners.run_method(&Listener::on_receive_data, buffer, bytes);
                continue;
            }

            std::unique_lock<Mutex> lg(m_lock);
            if (state() == State::DESTRUCTING){
                return;
            }

            if (bytes == 0){
                connection_lost_locked(lg, "Connection closed by server.", true);
                return;
            }

            if (bytes == SOCKET_ERROR){
                switch (error){
                case WSAEWOULDBLOCK:
                    break;
                default:
                    connection_lost_locked(lg, "Connection lost: WSA Error Code: " + std::to_string(error), true);
                    return;
                }
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }

    }


private:
    ThreadPool& m_thread_pool;
    std::atomic<SOCKET> m_socket{INVALID_SOCKET};

    std::string m_error;

    mutable Mutex m_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;
};



}
#endif
