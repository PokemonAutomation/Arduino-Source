/*  Client Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This file is completely untested!
 *
 */

#ifndef PokemonAutomation_ClientSocket_POSIX_H
#define PokemonAutomation_ClientSocket_POSIX_H

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "AbstractClientSocket.h"

namespace PokemonAutomation{



class ClientSocket_POSIX final : public AbstractClientSocket{
public:
    ClientSocket_POSIX()
        : m_socket(socket(AF_INET, SOCK_STREAM, 0))
    {
        fcntl(m_socket, F_SETFL, O_NONBLOCK);
    }

    virtual ~ClientSocket_POSIX(){
        close();
        if (m_thread.joinable()){
            m_thread.join();
        }
        if (m_socket != -1){
            ::close(m_socket);
        }
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
                &ClientSocket_POSIX::thread_loop,
                this, address, port
            );
        }catch (...){
            m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
            throw;
        }
    }


    virtual size_t send(const void* data, size_t bytes) override{
        if (m_socket == -1){
            return 0;
        }

        constexpr int BLOCK_SIZE = (int)1 << 30;
        const char* ptr = (const char*)data;
        size_t sent = 0;

        while (bytes > 0){
            size_t current = std::min<size_t>(bytes, BLOCK_SIZE);
            ssize_t current_sent = ::send(m_socket, ptr, (int)current, MSG_DONTWAIT);
            if (current_sent != -1){
                sent += current;
                if ((size_t)current_sent < current){
                    return sent;
                }
                ptr += current;
                bytes -= current;
                continue;
            }

            std::unique_lock<std::mutex> lg(m_lock);
            if (state() == State::DESTRUCTING){
                break;
            }

            int error = errno;
//            cout << "error = " << error << endl;
            switch (error){
            case EAGAIN:
//            case EWOULDBLOCK:
                break;
            default:
                m_error = "POSIX Error Code: " + std::to_string(error);
                return sent;
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }
        return sent;
    }


private:
    void thread_loop(const std::string& address, uint16_t port){
        try{
            thread_loop_internal(address, port);
        }catch (...){
            try{
                std::cout << "ClientSocket_POSIX(): An exception was thrown from the receive thread." << std::endl;
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

                if (::connect(m_socket, (struct sockaddr*)&server, sizeof(server)) != -1){
                    break;
                }

                int error = errno;
//                cout << "error = " << error << endl;

                switch (error){
                case EISCONN:
                    goto Connected;
                case EAGAIN:
                case EALREADY:
                case EINPROGRESS:
                    break;
                case ETIMEDOUT:
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
            ssize_t bytes;
            int error = 0;
            {
                std::unique_lock<std::mutex> lg(m_lock);
                State state = m_state.load(std::memory_order_relaxed);
                if (state == State::DESTRUCTING){
                    return;
                }
                bytes = ::recv(m_socket, buffer, BUFFER_SIZE, 0);
                if (bytes < 0){
                    error = errno;
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

            if (bytes < 0){
//                cout << "error = " << error << endl;
                switch (error){
                case EAGAIN:
//                case EWOULDBLOCK:
                    break;
                default:
                    std::unique_lock<std::mutex> lg(m_lock);
                    m_error = "POSIX Error Code: " + std::to_string(error);
                    return;
                }
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }

    }


private:
    const int m_socket;

    std::string m_error;

    mutable std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};


}
#endif
