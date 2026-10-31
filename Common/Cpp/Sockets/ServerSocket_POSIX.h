/*  Server Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_POSIX_H
#define PokemonAutomation_ServerSocket_POSIX_H

#include <string>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "AbstractServerSocket.h"

namespace PokemonAutomation{


class ServerSocket_POSIX final : public AbstractServerSocket{
public:
    ServerSocket_POSIX(ThreadPool& thread_pool)
        : m_thread_pool(thread_pool)
    {
        // Ignore SIGPIPE. Handle errors via errno instead
        signal(SIGPIPE, SIG_IGN);
    }

    virtual ~ServerSocket_POSIX(){
        close();
    }

    virtual void start(const std::string& address, uint16_t port) override{
        try{
            //  Use the plain dispatch() rather than dispatch_now_blocking():
            //  start() may be called from a thread that is itself running on
            //  this pool, and blocking here until a pool thread is free could
            //  deadlock a capped pool. The returned AsyncTask still lets
            //  close() join the listener thread. The dispatch happens under
            //  the lock so two "start()" calls cannot race into it.
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING
                || m_accept_thread){
                return;
            }
            m_accept_thread = m_thread_pool.dispatch([=, this]{
                accept_thread_loop(address, port);
            });
        }catch (...){
            m_listeners.run_method(&Listener::on_listen_finished, "Failed to start the listener thread.");
            throw;
        }
    }

    virtual void close_connection() override{
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

    virtual void close() noexcept override{
        AsyncTask accept_thread;
        AsyncTask client_thread;
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING
                && !m_accept_thread
                && !m_client_thread){
                //  Never started, or already fully closed.
                return;
            }
            m_close_requested = true;
            m_state.store(State::DESTRUCTING, std::memory_order_release);
            accept_thread = std::move(m_accept_thread);
            m_accept_thread = AsyncTask();
            client_thread = std::move(m_client_thread);
            m_client_thread = AsyncTask();
        }

        //  The threads notice the new state within a few milliseconds and
        //  close their own sockets.
        accept_thread.wait_and_ignore_exceptions();
        client_thread.wait_and_ignore_exceptions();

        std::lock_guard<Mutex> lg(m_lock);
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        m_close_requested = false;
    }

    virtual size_t send(const void* data, size_t bytes) override{
        int socket;
        {
            std::lock_guard<Mutex> lg(m_lock);
            socket =
                (m_state.load(std::memory_order_relaxed) == State::CONNECTED) ? m_client : -1;
        }
        if (socket == -1){
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

            constexpr int BLOCK_SIZE = (int)1 << 30;
            int current = std::min<int>((size_t)BLOCK_SIZE, bytes);
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


private:
    void accept_thread_loop(std::string address, uint16_t port){
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
            m_listeners.run_method(
                &Listener::on_listen_finished,
                "Failed to bind the socket: " + std::string(strerror(error))
            );
            return;
        }

        if (::listen(listen_socket, 16) == -1){
            int error = errno;
            ::close(listen_socket);
            m_listeners.run_method(
                &Listener::on_listen_finished,
                "Failed to listen on the socket: " + std::string(strerror(error))
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
                listen_socket = -1;
                m_state.store(State::LISTENING, std::memory_order_release);
            }
        }
        if (listen_socket != -1){
            ::close(listen_socket);
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
                if (state == State::DESTRUCTING || state == State::NOT_RUNNING){
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
                    try{
                        m_client_thread = m_thread_pool.dispatch([this]{
                            connection_thread_loop();
                        });
                    }catch (...){
                        //  The connection thread could not be started. Drop
                        //  this connection and keep listening.
                        ::close(m_client);
                        m_client = -1;
                        m_client_thread = AsyncTask();
                        m_state.store(State::LISTENING, std::memory_order_release);
                    }
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

    void connection_thread_loop(){
        m_listeners.run_method(&Listener::on_thread_start);

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
            ssize_t bytes;
            int error_no = 0;
            bool no_data;
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_state.load(std::memory_order_relaxed) != State::CONNECTED){
                    break;
                }
                bytes = ::recv(socket, buffer, BUFFER_SIZE, 0);
                no_data = (bytes == -1);
                if (no_data){
                    error_no = errno;
                    no_data = (error_no == EAGAIN);
#if EAGAIN != EWOULDBLOCK
                    no_data = no_data || (error_no == EWOULDBLOCK);
#endif
                }
            }

            if (bytes > 0){
                m_listeners.run_method(&Listener::on_receive_data, buffer, bytes);
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
                case EINTR:
                    continue;
                default:
                    error = "Receive error (errno " + std::to_string(error_no) + "): " + std::string(strerror(error_no));
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


private:
    ThreadPool& m_thread_pool;
    Mutex m_lock;

    int m_listen = -1;
    int m_client = -1;
    bool m_close_requested = false;

    AsyncTask m_accept_thread;
    AsyncTask m_client_thread;
};



}
#endif
