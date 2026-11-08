/*  Client Socket (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32

#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "ClientSocket_WinSocket.h"

#pragma comment(lib, "Ws2_32.lib")

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


void ClientSocket_WinSocket::close_and_wait() noexcept{
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
void ClientSocket_WinSocket::close() noexcept{
    //  Non-blocking teardown, safe to call from the receive thread. The
    //  blocking "close_and_wait()" waits for the receive thread to exit and so
    //  must not run from a listener callback; this instead just flips the
    //  state and releases the socket. The receive thread's next iteration sees
    //  DESTRUCTING (or the missing socket) and exits on its own. The socket is
    //  left DESTRUCTING; the destructor's close_and_wait() returns it to
    //  NOT_RUNNING.
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING && !m_thread){
            return;
        }
        m_state.store(State::DESTRUCTING, std::memory_order_release);
        close_socket_locked();
    }
    m_cv.notify_all();
}
void ClientSocket_WinSocket::connect(const std::string& address, uint16_t port){
    //  Complete any in-progress connection or a requested-but-unfinished
    //  close() before starting a new one. close() is non-blocking (it leaves
    //  the socket DESTRUCTING), so reconnecting on the same socket joins the
    //  old receive thread here to get back to NOT_RUNNING. No-op if the socket
    //  is already NOT_RUNNING.
    close_and_wait();

    std::string error;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
            return;
        }

        //  A socket whose previous connection has ended cannot be
        //  reconnected, so always start from a fresh socket.
        close_socket_locked();

        ensure_wsa_started();

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

    //  Use the plain dispatch() rather than dispatch_now_blocking():
    //  connect() may be called from a thread that is itself running on
    //  this pool, and blocking here until a pool thread is free could
    //  deadlock a capped pool. The returned AsyncTask still lets
    //  close_and_wait() join the receive thread.
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


size_t ClientSocket_WinSocket::send(const void* data, size_t bytes, std::chrono::milliseconds timeout){
    const char* ptr = (const char*)data;
    size_t sent = 0;

    //  "time_point::max()" (the infinite default) is never reached; using
    //  it also avoids overflowing "now() + timeout".
    const std::chrono::steady_clock::time_point deadline =
        (timeout == std::chrono::milliseconds::max())
            ? std::chrono::steady_clock::time_point::max()
            : std::chrono::steady_clock::now() + timeout;

    while (bytes > 0 && state() == State::CONNECTED){
        if (std::chrono::steady_clock::now() >= deadline){
            //  The deadline elapsed with bytes still unwritten; the
            //  connection is still fine, so just report the partial count.
            return sent;
        }
        SOCKET socket = m_socket.load(std::memory_order_acquire);
        if (socket == INVALID_SOCKET){
            return sent;
        }

        constexpr size_t BLOCK_SIZE = (size_t)1 << 30;
        int current = (int)std::min<size_t>(bytes, BLOCK_SIZE);
        int current_sent = ::send(socket, ptr, current, 0);
        if (current_sent != SOCKET_ERROR){
            //  A partial send is not an error: advance and keep looping
            //  until everything is written or the deadline/state ends
            //  the send, the same way the POSIX and Qt backends do.
            sent += current_sent;
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


void ClientSocket_WinSocket::ensure_wsa_started(){
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
void ClientSocket_WinSocket::close_socket_locked(){
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

void ClientSocket_WinSocket::connection_lost_locked(
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

void ClientSocket_WinSocket::thread_loop(const std::string& address, uint16_t port){
    try{
        thread_loop_internal(address, port);
    }catch (...){
        try{
            std::cout << "ClientSocket_WinSocket(): An exception was thrown from the receive thread." << std::endl;
        }catch (...){}
    }
}


void ClientSocket_WinSocket::thread_loop_internal(const std::string& address, uint16_t port){
    {
        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(address.c_str());

        //  Connect
        SOCKET socket;
        {
            std::lock_guard<Mutex> lg(m_lock);
            socket = m_socket.load(std::memory_order_acquire);
        }
        if (socket == INVALID_SOCKET){
            return;
        }

        bool in_progress = false;
        while (true){
            //  The lock must not be held across the connect wait: a
            //  connect to a host that drops SYNs can take the kernel
            //  tens of seconds to fail, and "close()" needs the lock to
            //  set DESTRUCTING. Hold it only for the state check (the
            //  same way the server's accept loop does it).
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_state.load(std::memory_order_relaxed) == State::DESTRUCTING){
                    return;
                }
            }

            if (!in_progress){
                int ret = ::connect(socket, (struct sockaddr*)&server, sizeof(server));
                if (ret != SOCKET_ERROR){
                    break;
                }

                int error = WSAGetLastError();
//                    cout << "error = " << error << endl;
                if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS){
                    std::unique_lock<Mutex> lg(m_lock);
                    if (error == WSAETIMEDOUT){
                        connection_lost_locked(lg, "Connection timed out.", false);
                    }else{
                        connection_lost_locked(lg, "WSA Error Code: " + std::to_string(error), false);
                    }
                    return;
                }
                in_progress = true;
            }

            //  Once the connect is in progress, calling connect() again
            //  is not safe on Windows: when the attempt fails, the
            //  second connect() returns WSAEINVAL instead of starting a
            //  new attempt. So wait for the socket to become writable
            //  and read the result with getsockopt(SO_ERROR).
            fd_set write_set;
            FD_ZERO(&write_set);
            FD_SET(socket, &write_set);
            timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 10000;
            int ret = ::select(0, nullptr, &write_set, nullptr, &tv);
            if (ret == SOCKET_ERROR){
                int error = WSAGetLastError();
                if (error != WSAEINTR){
                    std::unique_lock<Mutex> lg(m_lock);
                    connection_lost_locked(lg, "select() failed: WSA Error Code: " + std::to_string(error), false);
                    return;
                }
                continue;
            }
            if (ret == 0){
                continue;
            }

            int so_error = 0;
            int so_len = sizeof(so_error);
            if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&so_error, &so_len) == SOCKET_ERROR){
                so_error = WSAGetLastError();
            }
            if (so_error == 0){
                break;
            }
            std::unique_lock<Mutex> lg(m_lock);
            if (so_error == WSAETIMEDOUT){
                connection_lost_locked(lg, "Connection timed out.", false);
            }else if (so_error == WSAECONNREFUSED){
                connection_lost_locked(lg, "Connection refused.", false);
            }else{
                connection_lost_locked(lg, "WSA Error Code: " + std::to_string(so_error), false);
            }
            return;
        }

        {
            std::lock_guard<Mutex> lg(m_lock);
            //  "close()" may have set DESTRUCTING while the connect was
            //  in progress. Keep that state so the receive loop exits
            //  immediately instead of marking the socket CONNECTED.
            if (m_state.load(std::memory_order_relaxed) != State::DESTRUCTING){
                m_state.store(State::CONNECTED, std::memory_order_relaxed);
            }
        }
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




}
#endif
