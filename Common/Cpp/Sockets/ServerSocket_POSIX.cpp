/*  Server Socket (POSIX)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#if !_WIN32

#include <cerrno>
#include <csignal>
#include <cstring>
#include <chrono>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "ServerSocket_POSIX.h"

namespace PokemonAutomation{

size_t ServerSocketConnection_POSIX::send(
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
        //  socket. The socket is only written to while this object still
        //  owns it - once take_fd() has handed it off, the kernel may
        //  have reused the number for a different socket.
        if (!m_open.load(std::memory_order_relaxed)){
            return sent;
        }
        const int fd = m_client.load(std::memory_order_acquire);
        if (fd == -1){
            return sent;
        }
        if (std::chrono::steady_clock::now() >= deadline){
            //  The deadline elapsed with bytes still unwritten; the
            //  connection is still fine, so just report the partial count.
            return sent;
        }

        constexpr size_t BLOCK_SIZE = 1 << 30;
        size_t current = std::min(BLOCK_SIZE, bytes);
        ssize_t current_sent = ::send(fd, ptr, current, MSG_DONTWAIT | MSG_NOSIGNAL);
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
            //  EPIPE, ECONNRESET, EBADF, or any other error: the
            //  connection is gone; report the bytes written so far.
            return sent;
        }
    }

    return sent;
}
void ServerSocketConnection_POSIX::close(){
    if (m_close_requested.exchange(true)){
        return;
    }
    m_open.store(false, std::memory_order_release);
    //  Hand the CONNECTED state back before the close becomes visible
    //  to the peer: a client reconnecting after it observes this close
    //  must be accepted, not rejected by a state that is still winding
    //  down. (A no-op unless this connection is still the active one.)
    m_server->release_connection(this);
    //  The connection thread closes the socket when it exits: it is the
    //  only thread that reads it, and it is the only one that may close
    //  it. close() must not touch the socket at all - it can be called
    //  from a listener callback that runs on that thread, and by the
    //  time it runs the transport may already have closed the fd, in
    //  which case the kernel may have handed that number to a different
    //  socket (a ::shutdown() here would clobber the other socket,
    //  and a close() would leak the one this object owns). POSIX closes
    //  cleanly even with data still queued, so the thread's plain close
    //  is all this connection needs.
}
void ServerSocketConnection_POSIX::start_thread(ThreadPool& thread_pool){
    m_thread = thread_pool.dispatch([this]{
        thread_loop();
    });
}

void ServerSocketConnection_POSIX::thread_loop(){
    int socket = m_client.load(std::memory_order_acquire);

    if (!m_open.load(std::memory_order_relaxed)){
        //  Closed before this thread started (teardown, or the
        //  application via active_connection()): hand the state back and
        //  drop the socket; there is nothing to report.
        m_server->release_connection(this);
        const int owned = take_fd();
        if (owned != -1){
            ::close(owned);
        }
        return;
    }

    m_server->report_connect();

    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::string error;

    while (m_open.load(std::memory_order_relaxed)){
        //  The socket is non-blocking, so this returns immediately. It is
        //  deliberately called without holding any lock: the liveness
        //  check above and the result handling below are the only parts
        //  that need one, and no socket call should stretch it.
        ssize_t bytes = ::recv(socket, buffer, BUFFER_SIZE, 0);

        if (bytes > 0){
            //  The application may have closed the connection while we
            //  were reading: only dispatch the data if it is still open.
            if (m_open.load(std::memory_order_relaxed)){
                m_server->report_receive(buffer, bytes);
            }
            continue;
        }

        if (bytes == -1){
            int error_no = errno;
            if (error_no == EAGAIN){
                //  No data yet.
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
#if EAGAIN != EWOULDBLOCK
            if (error_no == EWOULDBLOCK){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
#endif
            if (error_no == EINTR){
                continue;
            }
            //  ECONNRESET, EBADF, or any other error: the connection is
            //  gone. (An application-side close() also lands here, via
            //  ::shutdown: it makes recv return 0 or fail, and the
            //  close_requested() check below suppresses the report.)
            error = "Receive error (errno " + std::to_string(error_no) + "): " + std::string(strerror(error_no));
        }else{
            //  The peer closed the connection.
            error = "Connection closed by client.";
        }
        break;
    }

    //  Hand the state back to the server, then make the connection
    //  inert. Either way, the object outlives the server. The socket is
    //  only closed while this object still owns it: close() may have
    //  already taken it (and shut it down), and no one else may.
    m_server->release_connection(this);
    const bool app_close = m_close_requested.load(std::memory_order_relaxed);
    m_open.store(false, std::memory_order_release);
    const int owned = take_fd();
    if (owned != -1){
        ::close(owned);
    }
    if (!app_close){
        m_server->report_disconnect(error);
    }
}






ServerSocket_POSIX::ServerSocket_POSIX(ThreadPool& thread_pool)
    : m_thread_pool(thread_pool)
{
    // Ignore SIGPIPE. Handle errors via errno instead
    signal(SIGPIPE, SIG_IGN);
}

void ServerSocket_POSIX::start(const std::string& address, uint16_t port){
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
        //  The slot's previous occupant (if any) is a failed attempt:
        //  start() is a no-op while a task is alive, so it has already
        //  exited and its join at replacement is instant. The live task
        //  is joined by close_and_wait().
        m_accept_thread = std::move(task);
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

void ServerSocket_POSIX::close_and_wait() noexcept{
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

    AsyncTask accept_thread;
    accept_thread = std::move(m_accept_thread);

    //  The threads notice the new state within a few milliseconds and
    //  close their own sockets.
    accept_thread.wait_and_ignore_exceptions();
    if (active){
        //  Join the connection thread before dropping m_active: the slot
        //  is what keeps the connection alive while its thread runs.
        std::static_pointer_cast<ServerSocketConnection_POSIX>(active)->wait_thread();
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        m_active = ConnectionPtr();
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        m_task_alive = false;
        m_closing = 0;
        m_cv.notify_all();
    }
}

void ServerSocket_POSIX::listen_failed(const std::string& error){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_task_alive = false;
    }
    m_listeners.run_method(&Listener::on_listen_finished, error);
}

void ServerSocket_POSIX::accept_thread_loop(std::string address, uint16_t port){
    int listen_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1){
        listen_failed("Failed to create the socket: " + std::string(strerror(errno)));
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
        listen_failed("Failed to bind the socket: " + std::string(strerror(error)));
        return;
    }

    if (::listen(listen_socket, 16) == -1){
        int error = errno;
        ::close(listen_socket);
        listen_failed("Failed to listen on the socket: " + std::string(strerror(error)));
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
            listen_socket = -1;
            m_state.store(State::LISTENING, std::memory_order_release);
        }
    }
    if (listen_socket != -1){
        ::close(listen_socket);
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
        int listen;
        ConnectionPtr old;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::DESTRUCTING ||
                state == State::NOT_RUNNING
            ){
                break;
            }
            listen = m_listen;
            //  Housekeeping: a previous connection that released the
            //  state (LISTENING) may not have returned from its thread
            //  yet. The slot anchors the connection's lifetime while its
            //  thread runs, so join the thread and drop the slot before
            //  accepting a new client.
            if (state == State::LISTENING && m_active){
                old = m_active;
            }
        }
        if (old){
            std::static_pointer_cast<ServerSocketConnection_POSIX>(old)->wait_thread();
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_active.get() == old.get()){
                    m_active = ConnectionPtr();
                }
            }
            continue;
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
            rejected = (state == State::CONNECTED);
        }

        if (rejected){
            //  A connection is already active: reject this client.
            ::close(client_socket);
            m_listeners.run_method(&Listener::on_connect_rejected);
            continue;
        }

        //  The state is LISTENING, so the housekeeping above has already
        //  joined the previous thread and cleared the slot.
        fcntl(client_socket, F_SETFL, O_NONBLOCK);
        //  Plain "new" + wrap, not make_shared: the constructor is
        //  private, and this call site is in the friend.
        auto conn = std::shared_ptr<ServerSocketConnection_POSIX>(
            new ServerSocketConnection_POSIX(*this, client_socket)
        );
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) != State::LISTENING){
                //  Torn down while polling. The shell owns the socket
                //  now, so hand it back through the shell instead of
                //  closing the local: the transport never started, so
                //  the shell's take_fd() still returns it.
                conn->mark_closed();
                const int owned = conn->take_fd();
                if (owned != -1){
                    ::close(owned);
                }
                break;
            }
            m_active = conn;
            m_state.store(State::CONNECTED, std::memory_order_release);
        }
        try{
            //  The thread captures "this" raw: m_active (set above, and
            //  dropped only after the thread is joined) keeps the
            //  connection alive for the whole thread.
            conn->start_thread(m_thread_pool);
        }catch (...){
            //  The connection thread could not be started. Drop this
            //  connection and keep listening, unless we were torn down
            //  in the meantime.
            {
                std::lock_guard<Mutex> lg(m_lock);
                if (m_state.load(std::memory_order_relaxed) == State::CONNECTED){
                    m_active = ConnectionPtr();
                    m_state.store(State::LISTENING, std::memory_order_release);
                }
            }
            conn->mark_closed();
            const int owned = conn->take_fd();
            if (owned != -1){
                ::close(owned);
            }
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

void ServerSocket_POSIX::report_connect(){
    //  m_active is still this connection for the whole life of its
    //  thread (the server drops it only after joining the thread), so
    //  reading the slot under m_lock finds it.
    ConnectionPtr conn = active_connection();
    if (conn){
        m_listeners.run_method(&Listener::on_connect, conn);
    }
}

void ServerSocket_POSIX::report_receive(const void* data, size_t bytes){
    ConnectionPtr conn = active_connection();
    if (conn){
        m_listeners.run_method(&Listener::on_receive_data, conn, data, bytes);
    }
}

void ServerSocket_POSIX::report_disconnect(const std::string& error){
    ConnectionPtr conn = active_connection();
    if (conn){
        m_listeners.run_method(&Listener::on_disconnect, conn, error);
    }
}

void ServerSocket_POSIX::release_connection(ServerSocketConnection_POSIX* self){
    std::lock_guard<Mutex> lg(m_lock);
    if (m_state.load(std::memory_order_relaxed) == State::CONNECTED
        && m_active.get() == self
    ){
        m_state.store(State::LISTENING, std::memory_order_release);
    }
    //  The m_active slot stays: it anchors the connection's lifetime
    //  while its thread runs. The server drops it only after it has
    //  joined the thread (a replacement or a teardown).
}




}
#endif
