/*  Client Socket (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_WinSocket_H
#define PokemonAutomation_ClientSocket_WinSocket_H

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <winsock.h>
#include "AbstractClientSocket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace PokemonAutomation{



class ClientSocket_WinSocket final : public AbstractClientSocket{
public:
    ClientSocket_WinSocket()
        : m_socket(::socket(AF_INET, SOCK_STREAM, 0))
    {
        u_long non_blocking = 1;
        if (ioctlsocket(m_socket, FIONBIO, &non_blocking)){
//            cout << "ioctlsocket() Failed" << endl;
            close_socket();
            return;
        }
    }

    virtual ~ClientSocket_WinSocket(){
        close();
        if (m_thread.joinable()){
            m_thread.join();
        }
        close_socket();
    }
    virtual void close() noexcept override{
        {
            std::lock_guard<std::mutex> lg1(m_lock);
            m_state.store(State::DESTRUCTING, std::memory_order_relaxed);
            m_cv.notify_all();
        }
    }

    virtual void connect(const std::string& address, uint16_t port) override{
        std::lock_guard<std::mutex> lg1(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
            return;
        }
        try{
            m_state.store(State::CONNECTING, std::memory_order_relaxed);
            m_thread = std::thread(
                &ClientSocket_WinSocket::thread_loop,
                this, address, port
            );
        }catch (...){
            m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
            throw;
        }
    }


    virtual size_t send(const void* data, size_t bytes) override{
        if (m_socket == INVALID_SOCKET){
            return 0;
        }

        constexpr int BLOCK_SIZE = (int)1 << 30;
        const char* ptr = (const char*)data;
        size_t sent = 0;

        while (bytes > 0 && state() == State::CONNECTED){
            size_t current = std::min<size_t>(bytes, BLOCK_SIZE);
            int current_sent = ::send(m_socket, ptr, (int)current, 0);
            if (current_sent != SOCKET_ERROR){
                sent += current;
                if ((size_t)current_sent < current){
                    return sent;
                }
                ptr += current;
                bytes -= current;
                continue;
            }

            int error = WSAGetLastError();
//            cout << "error = " << error << endl;

            std::unique_lock<std::mutex> lg(m_lock);
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
    void close_socket(){
        if (m_socket == INVALID_SOCKET){
            return;
        }
        int ret = closesocket(m_socket);
        if (ret == 0){
            return;
        }
        try{
            std::cout << "Failed closesocket(): WSA Error Code = " + std::to_string(ret) << std::endl;
        }catch (...){}
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
        m_listeners.run_method_unique(&Listener::on_thread_start);

        {
            std::unique_lock<std::mutex> lg(m_lock);

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

                if (::connect(m_socket, (struct sockaddr*)&server, sizeof(server)) != SOCKET_ERROR){
                    break;
                }

                int error = WSAGetLastError();
//                cout << "error = " << error << endl;

                switch (error){
                case WSAEISCONN:
                    goto Connected;
                case WSAEWOULDBLOCK:
                case WSAEINVAL:
                    break;
                case WSAETIMEDOUT:
                    m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
                    m_error = "Connection timed out.";
                    m_lock.unlock();
                    m_listeners.run_method_unique(&Listener::on_connect_finished, m_error);
                    return;
                default:
                    m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
                    m_error = "WSA Error Code: " + std::to_string(error);
                    m_lock.unlock();
                    m_listeners.run_method_unique(&Listener::on_connect_finished, m_error);
                    return;
                }

                m_cv.wait_for(lg, std::chrono::milliseconds(10));

            }

Connected:
            m_state.store(State::CONNECTED, std::memory_order_relaxed);
        }

        m_listeners.run_method_unique(&Listener::on_connect_finished, "");


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
                bytes = ::recv(m_socket, buffer, BUFFER_SIZE, 0);
                if (bytes == SOCKET_ERROR){
                    error = WSAGetLastError();
                }
            }

            if (bytes > 0){
                m_listeners.run_method_unique(&Listener::on_receive_data, buffer, bytes);
                continue;
            }

            std::unique_lock<std::mutex> lg(m_lock);
            if (state() == State::DESTRUCTING){
                return;
            }

            if (bytes == SOCKET_ERROR){
//                cout << "error = " << error << endl;
                switch (error){
                case WSAEWOULDBLOCK:
                    break;
                default:
                    m_error = "WSA Error Code: " + std::to_string(error);
                    return;
                }
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }

    }


private:
    const SOCKET m_socket;

    std::string m_error;

    mutable std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};



}
#endif
