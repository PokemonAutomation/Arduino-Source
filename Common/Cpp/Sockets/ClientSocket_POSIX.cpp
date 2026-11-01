/*  Client Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#if !_WIN32

#include <poll.h>
#include <signal.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "ClientSocket_POSIX.h"

namespace PokemonAutomation{


ClientSocket_POSIX::ClientSocket_POSIX(ThreadPool& thread_pool)
    : m_thread_pool(thread_pool)
{
    // Ignore SIGPIPE. Handle errors via errno instead
    signal(SIGPIPE, SIG_IGN);
}
void ClientSocket_POSIX::close() noexcept{
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

void ClientSocket_POSIX::connect(const std::string& address, uint16_t port){
    std::string error;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
            return;
        }

        //  A socket whose previous connection has ended cannot be
        //  reconnected, so always start from a fresh socket.
        close_socket_locked();

        int socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket == -1){
            error = "socket() failed: " + std::string(strerror(errno));
        }else{
            int flags = fcntl(socket, F_GETFL, 0);
            if (flags == -1 || fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1){
                error = "fcntl() failed: " + std::string(strerror(errno));
                ::close(socket);
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

    //  Use the plain dispatch() rather than dispatch_now_blocking():
    //  connect() may be called from a thread that is itself running on
    //  this pool, and blocking here until a pool thread is free could
    //  deadlock a capped pool. The returned AsyncTask still lets
    //  close() join the receive thread.
    try{
        m_thread = m_thread_pool.dispatch([=, this]{
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
void ClientSocket_POSIX::close_socket_locked(){
    int socket = m_socket.exchange(-1, std::memory_order_acquire);
    if (socket == -1){
        return;
    }
    if (::close(socket) != 0){
        try{
            std::cout << "Failed close(): errno = " << errno << " (" << strerror(errno) << ")" << std::endl;
        }catch (...){}
    }
}

size_t ClientSocket_POSIX::send(const void* data, size_t bytes){
    const char* ptr = (const char*)data;
    size_t sent = 0;

    while (bytes > 0 && state() == State::CONNECTED){
        int socket = m_socket.load(std::memory_order_acquire);
        if (socket == -1){
            return sent;
        }

        constexpr int BLOCK_SIZE = (int)1 << 30;
        size_t current = std::min<size_t>(bytes, BLOCK_SIZE);
        ssize_t current_sent = ::send(socket, ptr, (int)current, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (current_sent != -1){
            sent += current_sent;
            if ((size_t)current_sent < current){
                return sent;
            }
            ptr += current_sent;
            bytes -= current_sent;
            continue;
        }

        int error = errno;
//            cout << "error = " << error << endl;

        std::unique_lock<Mutex> lg(m_lock);
        if (state() == State::DESTRUCTING){
            break;
        }

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

void ClientSocket_POSIX::connection_lost_locked(
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

void ClientSocket_POSIX::thread_loop(std::string address, uint16_t port){
    try{
        thread_loop_internal(std::move(address), port);
    }catch (...){
        try{
            std::cout << "ClientSocket_POSIX(): An exception was thrown from the receive thread." << std::endl;
        }catch (...){}
    }
}
void ClientSocket_POSIX::thread_loop_internal(std::string address, uint16_t port){
    m_listeners.run_method(&Listener::on_thread_start);

    {
        std::unique_lock<Mutex> lg(m_lock);

        sockaddr_in server;
        std::memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(address.c_str());

        //  Connect
        int socket = m_socket.load(std::memory_order_acquire);
        if (socket == -1){
            return;
        }

        bool in_progress = false;
        while (true){
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING){
                return;
            }

            if (!in_progress){
                int ret = ::connect(socket, (struct sockaddr*)&server, sizeof(server));
                if (ret == 0){
                    break;
                }

                int error = errno;
//                    cout << "error = " << error << endl;
                if (error != EAGAIN && error != EALREADY && error != EINPROGRESS){
                    if (error == ETIMEDOUT){
                        connection_lost_locked(lg, "Connection timed out.", false);
                    }else{
                        connection_lost_locked(lg, "Connect error (errno " + std::to_string(error) + "): " + std::string(strerror(error)), false);
                    }
                    return;
                }
                in_progress = true;
            }

            //  While the connect is in progress, calling connect() again
            //  returns EISCONN, so instead wait for the socket to become
            //  writable, then check SO_ERROR for the result.
            pollfd pf;
            pf.fd = socket;
            pf.events = POLLOUT;
            pf.revents = 0;
            int pret = poll(&pf, 1, 10);
            if (pret == -1){
                if (errno != EINTR){
                    connection_lost_locked(lg, "poll() error: " + std::string(strerror(errno)), false);
                    return;
                }
                continue;
            }
            if (pret == 0){
                continue;
            }

            int so_error = 0;
            socklen_t slen = sizeof(so_error);
            if (getsockopt(socket, SOL_SOCKET, SO_ERROR, &so_error, &slen) != 0){
                so_error = errno;
            }
            if (so_error == 0){
                break;
            }
            if (so_error == ETIMEDOUT){
                connection_lost_locked(lg, "Connection timed out.", false);
            }else if (so_error == ECONNREFUSED){
                connection_lost_locked(lg, "Connection refused.", false);
            }else{
                connection_lost_locked(lg, "Connect error (errno " + std::to_string(so_error) + "): " + std::string(strerror(so_error)), false);
            }
            return;
        }

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

            int socket = m_socket.load(std::memory_order_acquire);
            if (socket == -1){
                return;
            }

            bytes = ::recv(socket, buffer, BUFFER_SIZE, 0);
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

        if (bytes == 0){
            connection_lost_locked(lg, "Connection closed by server.", true);
            return;
        }

        if (bytes == -1){
            switch (error){
            case EAGAIN:
#if EAGAIN != EWOULDBLOCK
            case EWOULDBLOCK:
#endif
                break;
            default:
                connection_lost_locked(lg, "Receive error (errno " + std::to_string(error) + "): " + std::string(strerror(error)), true);
                return;
            }
        }

        m_cv.wait_for(lg, std::chrono::milliseconds(1));
    }

}



}
#endif
