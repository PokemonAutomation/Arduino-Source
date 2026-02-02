/*  Client Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This file is mostly untested!
 *
 */

#ifndef PokemonAutomation_ClientSocket_POSIX_H
#define PokemonAutomation_ClientSocket_POSIX_H

#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <signal.h>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "AbstractClientSocket.h"

namespace PokemonAutomation{



class ClientSocket_POSIX final : public AbstractClientSocket{
public:
    ClientSocket_POSIX()
        : m_socket(socket(AF_INET, SOCK_STREAM, 0)){
        // Ignore SIGPIPE. Handle errors via errno instead
        signal(SIGPIPE, SIG_IGN);

        if (m_socket != -1){
            fcntl(m_socket, F_SETFL, O_NONBLOCK);
        }
    }

    virtual ~ClientSocket_POSIX(){
        close();
        m_thread.join();
        close_socket();
    }

    virtual void close() noexcept override{
        {
            std::lock_guard<Mutex> lg1(m_lock);
            m_state.store(State::DESTRUCTING, std::memory_order_relaxed);
            m_cv.notify_all();
        }
    }

    virtual void connect(const std::string& address, uint16_t port) override{
        std::lock_guard<Mutex> lg1(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
            return;
        }
        try{
            m_state.store(State::CONNECTING, std::memory_order_relaxed);
            m_thread = Thread([addr = std::move(address), port, this] {
                thread_loop(std::move(addr), port);
            });
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

        while (bytes > 0 && state() == State::CONNECTED){
            size_t current = std::min<size_t>(bytes, BLOCK_SIZE);
            ssize_t current_sent = ::send(m_socket, ptr, (int)current, MSG_DONTWAIT | MSG_NOSIGNAL);
            if (current_sent != -1){
                sent += current_sent;
                if ((size_t)current_sent < current){
                    return sent;
                }
                ptr += current_sent;
                bytes -= current_sent;
                continue;
            }

            std::unique_lock<Mutex> lg(m_lock);
            if (state() == State::DESTRUCTING){
                break;
            }

            int error = errno;
//            cout << "error = " << error << endl;
            switch (error){
            case EAGAIN:
#if EAGAIN != EWOULDBLOCK
            case EWOULDBLOCK:
#endif
                break;
            case EPIPE:
            case ECONNRESET:
            case ENOTCONN:
                m_error = "Connection closed: " + std::string(strerror(error));
                return sent;
            default:
                m_error = "Send error (errno " + std::to_string(error) + "): " + std::string(strerror(error));
                return sent;
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }
        return sent;
    }


private:
    void close_socket(){
        if (m_socket == -1){
            return;
        }
        int ret = ::close(m_socket);
        if (ret != 0){
            try{
                std::cout << "Failed close(): errno = " << errno << " (" << strerror(errno) << ")" << std::endl;
            }catch (...){}
        }
    }

    void thread_loop(std::string address, uint16_t port){
        try{
            thread_loop_internal(std::move(address), port);
        }catch (...){
            try{
                std::cout << "ClientSocket_POSIX(): An exception was thrown from the receive thread." << std::endl;
            }catch (...){}
        }
    }

    void thread_loop_internal(std::string address, uint16_t port){
        m_listeners.run_method(&Listener::on_thread_start);

        {
            std::unique_lock<Mutex> lg(m_lock);

            sockaddr_in server;
            std::memset(&server, 0, sizeof(server));
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
                    lg.unlock();
                    m_listeners.run_method(&Listener::on_connect_finished, m_error);
                    return;
                case ECONNREFUSED:
                    m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
                    m_error = "Connection refused.";
                    lg.unlock();
                    m_listeners.run_method(&Listener::on_connect_finished, m_error);
                    return;
                default:
                    m_state.store(State::NOT_RUNNING, std::memory_order_relaxed);
                    m_error = "Connect error (errno " + std::to_string(error) + "): " + std::string(strerror(error));
                    lg.unlock();
                    m_listeners.run_method(&Listener::on_connect_finished, m_error);
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
            ssize_t bytes;
            int error = 0;
            {
                std::unique_lock<Mutex> lg(m_lock);
                State state = m_state.load(std::memory_order_relaxed);
                if (state == State::DESTRUCTING){
                    return;
                }
                bytes = ::recv(m_socket, buffer, BUFFER_SIZE, 0);
//                cout << "error = " << error << endl;
                if (bytes == -1){
                    error = errno;
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

//            cout << "error = " << error << endl;
            if (bytes == -1){
                switch (error){
                case EAGAIN:
#if EAGAIN != EWOULDBLOCK
                case EWOULDBLOCK:
#endif
                    break;
                case ECONNRESET:
                case ENOTCONN:
                case EPIPE:
                    m_error = "Receive error (errno " + std::to_string(error) + "): " + std::string(strerror(error));
                    return;
                default:
                    m_error = "Receive error (errno " + std::to_string(error) + "): " + std::string(strerror(error));
                    return;
                }
            }

            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }

    }


private:
    const int m_socket;

    std::string m_error;

    mutable Mutex m_lock;
    ConditionVariable m_cv;
    Thread m_thread;
};


}
#endif
