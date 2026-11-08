/*  Server Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <algorithm>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include "Common/Qt/QtThreadPool.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "ServerSocket_Qt.h"

namespace PokemonAutomation{

void ServerSocket_Qt::start(const std::string& address, uint16_t port){
    bool starting;
    {
        std::lock_guard<Mutex> lg(m_lock);
        starting =
            (m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING)
            && m_server == nullptr
            && !m_starting;
        //  Only claim the "starting" flag when we actually start: a
        //  rejected call (server already running or already started) must
        //  not leave "m_starting" true - "close_and_wait()" waits on
        //  "!m_starting", so a stale flag would deadlock the destructor.
        //  (This happens when a test's start-retry loop calls start() on
        //  a server whose first listen failed: "m_server" is set even on
        //  a failed listen, so the retry is rejected here.)
        if (starting){
            m_starting = true;
        }
    }
    if (!starting){
        return;
    }

    m_listen_error = "";
    QTcpServer* server = nullptr;
    try{
        server = static_cast<QTcpServer*>(
            GlobalThreadPools::qt_event_threadpool().add_object(
                [this, address, port]{
                    return make_server(address, port);
                }
            )
        );
    }catch (...){
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_starting = false;
            m_cv.notify_all();
        }
        m_listeners.run_method(&Listener::on_listen_finished, "Failed to start the listener.");
        throw;
    }
    bool listening = false;
    bool destructing = false;
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_starting = false;
        destructing =
            (m_state.load(std::memory_order_relaxed) == State::DESTRUCTING);
        m_server = server;
        if (destructing){
            //  "close_and_wait()" is already in progress: it will destroy the
            //  server we just created. Leave the state for it.
        }else{
            listening = m_listen_error.empty();
            m_state.store(
                listening ? State::LISTENING : State::NOT_RUNNING,
                std::memory_order_release
            );
        }
        m_cv.notify_all();
    }

    if (!destructing && !listening){
        //  The listen failed. Destroy the server now, leaving the object
        //  exactly as it was before this start() (no "m_server", state
        //  NOT_RUNNING), so the caller's start-retry loop can immediately
        //  try another port. The other backends behave the same: a failed
        //  listen leaves no listener behind. If "close_and_wait()" raced us, it has
        //  already claimed "m_server", and the pool's lookup makes the
        //  second destruction a no-op.
        GlobalThreadPools::qt_event_threadpool().remove_object(server);
        std::lock_guard<Mutex> lg(m_lock);
        m_server = nullptr;
    }

    m_listeners.run_method(&Listener::on_listen_finished, m_listen_error);
}

void ServerSocket_Qt::close_connection(){
    QThread* event_thread;
    {
        std::lock_guard<Mutex> lg(m_lock);
        event_thread = m_event_thread;
        if (m_server == nullptr){
            //  Not running.
            return;
        }
    }

    //  Cancel the in-flight "send()" calls so their callers return
    //  promptly with the best-effort byte count. (The connection itself
    //  is closed by the queued call below.)
    cancel_inflight_sends();

    QSharedPointer<CloseCompletion> completion = QSharedPointer<CloseCompletion>::create();
    emit internal_close_connection(completion);

    //  If called from a thread other than the server's event thread, the
    //  call is queued: wait until it has run, so the state is up to date
    //  when this returns (same as the POSIX backend). If called from the
    //  event thread (e.g. a listener callback), the call ran directly and
    //  this does not block.
    //
    //  The shared ownership makes the timeout safe: if it expires, the
    //  queued call's own copy keeps the completion alive until it runs.
    if (QThread::currentThread() != event_thread){
        std::unique_lock<Mutex> lg(completion->lock);
        completion->cv.wait_for(
            lg,
            std::chrono::seconds(5),
            [&completion]{ return completion->done; }
        );
    }
}
void ServerSocket_Qt::close_and_wait() noexcept{
    QTcpServer* server;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_server == nullptr && !m_starting){
            //  Never started, or already closed.
            return;
        }
        m_close_requested = true;
        m_state.store(State::DESTRUCTING, std::memory_order_release);
    }

    //  Cancel the in-flight "send()" calls so their callers return
    //  promptly with the best-effort byte count.
    cancel_inflight_sends();

    //  Wait for an in-flight "start()" to finish so we destroy the
    //  server it creates.
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [this]{
            return !m_starting;
        });
        server = m_server;
        m_server = nullptr;
        m_socket = nullptr;
    }
    if (server == nullptr){
        //  "start()" failed or was never reached: nothing to destroy.
        std::lock_guard<Mutex> lg(m_lock);
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        m_close_requested = false;
        return;
    }

    //  Destroy the server on its event thread. This blocks until the
    //  destruction has happened.
    GlobalThreadPools::qt_event_threadpool().remove_object(server);

    std::lock_guard<Mutex> lg(m_lock);
    m_state.store(State::NOT_RUNNING, std::memory_order_release);
    m_close_requested = false;
}

size_t ServerSocket_Qt::send(const void* data, size_t bytes, std::chrono::milliseconds timeout){
    QTcpSocket* socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_state.load(std::memory_order_relaxed) == State::CONNECTED
            ? m_socket
            : nullptr;
    }
    if (socket == nullptr){
        return 0;
    }

    QSharedPointer<SendData> send_data = QSharedPointer<SendData>::create();
    send_data->packet.assign((const char*)data, bytes);
    //  "time_point::max()" (the infinite default) is never reached; using
    //  it also avoids overflowing "now() + timeout".
    send_data->deadline =
        (timeout == std::chrono::milliseconds::max())
            ? std::chrono::steady_clock::time_point::max()
            : std::chrono::steady_clock::now() + timeout;

    //  Register the send atomically with respect to
    //  "close_and_wait()"/"close_connection()": "close_and_wait()" sets the
    //  state to DESTRUCTING and detaches the socket under this same lock
    //  before it cancels the in-flight sends. The packet copy above can
    //  take a while for large payloads, so without this re-check a send
    //  could register *after*
    //  "close_and_wait()" already cancelled the (empty) list and queued the
    //  server's destruction: its "internal_send" call would then be
    //  posted after the destruction event, Qt would silently discard it,
    //  and the wait below (unbounded for the infinite default timeout)
    //  would never end. With the re-check, a send either registers
    //  before the cancel (and gets cancelled, waking the wait below) or
    //  sees the detached socket and returns without queueing anything.
    bool registered;
    {
        std::lock_guard<Mutex> lg(m_lock);
        registered =
            m_state.load(std::memory_order_relaxed) == State::CONNECTED
            && m_socket != nullptr;
        if (registered){
            m_inflight_sends.push_back(send_data);
        }
    }
    if (!registered){
        return 0;
    }

    emit internal_send(send_data);

    //  Wait for the server's event thread to finish writing the packet,
    //  or for "close_and_wait()"/"close_connection()" to cancel the send. A
    //  finite "timeout" waits for the deadline plus 1 s of queueing
    //  margin: the write loop stops at the deadline, so anything beyond
    //  that is just the queued call waiting to start.
    //
    //  The infinite default waits without a time bound. The registration
    //  above is atomic with the "close_and_wait()"/"close_connection()" detach, so
    //  "done" is set by either the event-thread write loop (which stops
    //  at the deadline, on a socket error, or when Qt's userspace buffer
    //  accepts the packet; it runs before the queued server destruction,
    //  since the event loop is FIFO) or by "close_and_wait()"/"close_connection()"
    //  cancelling the registered send - even when Qt discards the queued
    //  write loop with the server - so the wait always ends. The shared
    //  ownership keeps the packet alive until the queued call runs (or
    //  is discarded with the server).
    {
        std::unique_lock<Mutex> lg(send_data->lock);
        if (timeout == std::chrono::milliseconds::max()){
            send_data->cv.wait(
                lg,
                [&send_data]{ return send_data->done; }
            );
        }else{
            send_data->cv.wait_for(
                lg,
                timeout + std::chrono::milliseconds(1000),
                [&send_data]{ return send_data->done; }
            );
        }
    }

    return send_data->bytes_sent;
}

void ServerSocket_Qt::cancel_inflight_sends(){
    std::vector<QSharedPointer<SendData>> cancelled;
    {
        std::lock_guard<Mutex> lg(m_lock);
        cancelled = std::move(m_inflight_sends);
    }
    for (const auto& send_data : cancelled){
        {
            std::lock_guard<Mutex> lg(send_data->lock);
            send_data->cancelled = true;
            send_data->done = true;
        }
        send_data->cv.notify_all();
    }
}

void ServerSocket_Qt::on_new_connection(){
    QTcpServer* server;
    {
        std::lock_guard<Mutex> lg(m_lock);
        server = m_server;
    }
    if (server == nullptr){
        return;
    }

    while (true){
        QTcpSocket* sock = server->nextPendingConnection();
        if (sock == nullptr){
            break;
        }
        //  The connection socket lives with the server, so
        //  "close_and_wait()" destroys it on the event thread.
        sock->setParent(server);

        bool accepted;
        bool rejected;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::CONNECTED){
                //  A client is already connected: reject this one.
                accepted = false;
                rejected = true;
            }else if (state != State::LISTENING){
                //  The server is shutting down.
                accepted = false;
                rejected = false;
            }else{
                m_socket = sock;
                m_close_requested = false;
                m_state.store(State::CONNECTED, std::memory_order_release);
                accepted = true;
                rejected = false;
            }
        }

        if (!accepted){
            if (rejected){
                m_listeners.run_method(&Listener::on_connect_rejected);
            }
            sock->abort();
            //  The socket is never used again: delete it, so a peer that
            //  repeatedly connects (or keeps trying while another client
            //  is connected) cannot accumulate one QTcpSocket per attempt
            //  - they would otherwise stay children of the server until
            //  the whole server is destroyed.
            sock->deleteLater();
            continue;
        }

        QObject::connect(
            sock, &QAbstractSocket::readyRead,
            sock, [this, sock]{
                //  readyRead only fires once per delivery, and more than
                //  one buffer's worth of data may be waiting, so read
                //  until the socket has no more data available.
                while (true){
                    qint64 available = sock->bytesAvailable();
                    if (available <= 0){
                        break;
                    }

                    constexpr size_t BUFFER_SIZE = 4096;
                    char buffer[BUFFER_SIZE];
                    qint64 bytes = sock->read(buffer, std::min<qint64>(BUFFER_SIZE, available));
                    if (bytes <= 0){
                        break;
                    }

                    m_listeners.run_method(&Listener::on_receive_data, buffer, bytes);
                }
            }
        );
        QObject::connect(
            sock, &QAbstractSocket::disconnected,
            sock, [this, sock]{
                connection_lost(sock, "Connection closed by client.");
            }
        );
        QObject::connect(
            sock, &QAbstractSocket::errorOccurred,
            sock, [this, sock](QAbstractSocket::SocketError){
                connection_lost(sock, sock->errorString().toStdString());
            }
        );

        m_listeners.run_method(&Listener::on_connect);
    }
}

void ServerSocket_Qt::connection_lost(QTcpSocket* sock, const std::string& error_message){
    bool report;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_socket != sock){
            //  Already replaced or closed; the first callback wins.
            return;
        }
        report =
            !m_close_requested
            && m_state.load(std::memory_order_relaxed) == State::CONNECTED;
        m_socket = nullptr;
        if (m_state.load(std::memory_order_relaxed) == State::CONNECTED){
            m_state.store(State::LISTENING, std::memory_order_release);
        }
    }

    if (report){
        m_listeners.run_method(&Listener::on_disconnect, error_message);
    }

    //  The socket is no longer tracked (m_socket was nulled above, and the
    //  first callback for a socket wins), so it can be released: schedule
    //  its deletion on the event thread (this runs on it). A duplicate
    //  signal for the same socket returned above and does not schedule a
    //  second deletion; deleteLater() is idempotent either way.
    sock->deleteLater();
}

void ServerSocket_Qt::on_close_connection(QSharedPointer<CloseCompletion> completion){
    QTcpSocket* sock;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_socket != nullptr){
            m_close_requested = true;
            if (m_state.load(std::memory_order_relaxed) != State::DESTRUCTING){
                //  The server keeps running, so it goes back to
                //  LISTENING. The destructor's close_and_wait() may have set
                //  DESTRUCTING: leave the state for it.
                m_state.store(State::LISTENING, std::memory_order_release);
            }
            sock = m_socket;
            m_socket = nullptr;
        }else{
            sock = nullptr;
        }
    }
    if (sock != nullptr){
        sock->abort();
        //  The socket is no longer tracked (m_socket was nulled above):
        //  schedule its deletion on the event thread. The
        //  disconnected/errorOccurred signals that abort() may still emit
        //  go through connection_lost(), which returns early because
        //  m_socket is already null, so the socket is not scheduled twice.
        sock->deleteLater();
    }

    {
        std::lock_guard<Mutex> lg(completion->lock);
        completion->done = true;
    }
    completion->cv.notify_all();
}

void ServerSocket_Qt::on_send(QSharedPointer<SendData> send_data){
    QTcpSocket* sock;
    {
        std::lock_guard<Mutex> lg(m_lock);
        sock = m_socket;
    }

    {
        std::lock_guard<Mutex> lg(send_data->lock);
        if (send_data->cancelled){
            //  "close_and_wait()"/"close_connection()" already cancelled this
            //  send and woken its caller; finish without writing.
            return;
        }
    }

    size_t bytes_sent = 0;
    if (sock != nullptr){
        const char* ptr = send_data->packet.data();
        size_t remaining = send_data->packet.size();
        //  The deadline keeps a client that never reads from pinning this
        //  event thread forever (which would also block
        //  "close_and_wait()").
        while (
            remaining > 0
            && sock->state() == QAbstractSocket::ConnectedState
            && std::chrono::steady_clock::now() < send_data->deadline
        ){
            {
                std::lock_guard<Mutex> lg(send_data->lock);
                if (send_data->cancelled){
                    //  Cancelled mid-write; the caller has already been
                    //  woken with the count written so far.
                    break;
                }
            }
            qint64 current_sent = sock->write(ptr, remaining);
            if (current_sent <= 0){
                //  The kernel's send buffer is full. Wait for some of it
                //  to drain, then try again.
                sock->waitForBytesWritten(10);
                continue;
            }
            ptr += current_sent;
            remaining -= (size_t)current_sent;
            //  Track the running count, so a close that cancels this
            //  send mid-write reports the bytes written so far.
            {
                std::lock_guard<Mutex> lg(send_data->lock);
                send_data->bytes_sent = send_data->packet.size() - remaining;
            }
        }
        bytes_sent = send_data->packet.size() - remaining;
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        //  "close_and_wait()"/"close_connection()" may already have claimed this
        //  send; only remove it if it still owns its slot.
        m_inflight_sends.erase(
            std::remove(
                m_inflight_sends.begin(),
                m_inflight_sends.end(),
                send_data
            ),
            m_inflight_sends.end()
        );
    }

    {
        std::lock_guard<Mutex> lg(send_data->lock);
        send_data->bytes_sent = bytes_sent;
        send_data->done = true;
    }
    send_data->cv.notify_all();
}

std::unique_ptr<QTcpServer> ServerSocket_Qt::make_server(const std::string& address, uint16_t port){
    auto server = std::make_unique<QTcpServer>();
    if (!server->listen(QHostAddress(QString::fromStdString(address)), port)){
        std::lock_guard<Mutex> lg(m_lock);
        m_listen_error = server->errorString().toStdString();
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        m_event_thread = QThread::currentThread();
    }

    QObject::connect(
        this, &ServerSocket_Qt::internal_send,
        server.get(), [this](QSharedPointer<SendData> send_data){
            on_send(send_data);
        }
    );
    QObject::connect(
        this, &ServerSocket_Qt::internal_close_connection,
        server.get(), [this](QSharedPointer<CloseCompletion> completion){
            on_close_connection(completion);
        }
    );
    QObject::connect(
        server.get(), &QTcpServer::newConnection,
        server.get(), [this]{
            on_new_connection();
        }
    );

    return server;
}

}
