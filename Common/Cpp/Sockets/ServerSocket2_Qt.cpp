/*  Server Socket 2 (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <algorithm>
#include <QTcpServer>
#include <QTcpSocket>
#include "Common/Qt/QtThreadPool.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "ServerSocket2_Qt.h"

namespace PokemonAutomation{

size_t ServerSocket2Connection_Qt::send(
    const void* data,
    size_t bytes,
    std::chrono::milliseconds timeout
){
    if (!m_open.load(std::memory_order_relaxed)){
        return 0;
    }

    QSharedPointer<SendData> send_data = QSharedPointer<SendData>::create();
    send_data->packet.assign((const char*)data, bytes);
    //  The queued call must keep the shell alive until it runs (or is
    //  discarded with the server), even if the server and the listeners
    //  have already dropped their references.
    send_data->conn = shared_from_this();
    //  "time_point::max()" (the infinite default) is never reached; using
    //  it also avoids overflowing "now() + timeout".
    send_data->deadline =
        (timeout == std::chrono::milliseconds::max())
            ? std::chrono::steady_clock::time_point::max()
            : std::chrono::steady_clock::now() + timeout;

    //  Register the send atomically with respect to
    //  "close()"/"close_for_teardown()": they flip "m_open" under this
    //  same lock before they drain "m_inflight". The packet copy above
    //  can take a while for large payloads, so the m_open check happens
    //  under the lock, not before it. Holding the lock through the emit
    //  as well is what makes the server alive for it: the teardown that
    //  would destroy the server takes this lock first, and any drain
    //  that runs later still finds this send in the list and cancels it
    //  (waking the wait below), even if the queued write is then
    //  discarded with the server.
    bool registered;
    {
        std::lock_guard<RecursiveMutex> lg(m_lock);
        registered = m_open.load(std::memory_order_relaxed);
        if (registered){
            m_inflight.push_back(send_data);
            m_server->queue_send(send_data);
        }
    }
    if (!registered){
        return 0;
    }

    //  Wait for the server's event thread to finish writing the packet,
    //  or for "close()"/"close_for_teardown()" to cancel the send. A
    //  finite "timeout" waits for the deadline plus 1 s of queueing
    //  margin: the write loop stops at the deadline, so anything beyond
    //  that is just the queued call waiting to start.
    //
    //  The infinite default waits without a time bound. The registration
    //  above is atomic with the close's detach, so "done" is set by
    //  either the event-thread write loop (which stops at the deadline,
    //  on a socket error, or when Qt's userspace buffer accepts the
    //  packet; it runs before the queued server destruction, since the
    //  event loop is FIFO) or by the close cancelling the registered
    //  send - even when Qt discards the queued write loop with the
    //  server - so the wait always ends. The shared ownership keeps the
    //  packet (and the shell) alive until the queued call runs (or is
    //  discarded with the server).
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

void ServerSocket2Connection_Qt::close(){
    //  "m_lock" spans the whole close: "close_for_teardown()" takes it
    //  before the server is destroyed, so if we claim the close the
    //  server is alive for the queued call below - and if we do not
    //  claim it, someone already did.
    std::lock_guard<RecursiveMutex> lg(m_lock);
    if (m_close_requested.exchange(true)){
        return;
    }
    m_open.store(false, std::memory_order_release);
    //  Cancel the in-flight "send()" calls so their callers return
    //  promptly with the best-effort byte count. The connection itself
    //  is closed by the queued call below, which "close()" does not
    //  wait for (the state updates when it runs on the event thread).
    drain_inflight_sends();
    m_server->close_connection_on();
}

void ServerSocket2Connection_Qt::close_for_teardown(){
    std::lock_guard<RecursiveMutex> lg(m_lock);
    m_close_requested.store(true, std::memory_order_release);
    m_open.store(false, std::memory_order_release);
    drain_inflight_sends();
}


void ServerSocket2_Qt::start(const std::string& address, uint16_t port){
    bool starting;
    {
        std::lock_guard<Mutex> lg(m_lock);
        starting =
            (m_state.load(std::memory_order_relaxed) == State::NOT_RUNNING)
            && m_server == nullptr
            && !m_starting;
        //  Only claim the "starting" flag when we actually start: a
        //  rejected call (server already running, or another start()
        //  still in flight) must not leave "m_starting" true -
        //  "close_and_wait()" waits on "!m_starting", so a stale flag
        //  would deadlock the destructor.
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
            //  "close_and_wait()" is already in progress: it will destroy
            //  the server we just created. Leave the state for it.
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
        //  listen leaves no listener behind. If "close_and_wait()" raced
        //  us, it has already claimed "m_server", and the pool's lookup
        //  makes the second destruction a no-op.
        GlobalThreadPools::qt_event_threadpool().remove_object(server);
        std::lock_guard<Mutex> lg(m_lock);
        m_server = nullptr;
    }

    m_listeners.run_method(&Listener::on_listen_finished, m_listen_error);
}

void ServerSocket2_Qt::close_connection_on(){
    //  Called from the shell's "close()" with the shell's lock held:
    //  only enqueue the abort; the shell does not wait for it.
    emit internal_close_connection();
}

void ServerSocket2_Qt::close_and_wait() noexcept{
    ConnectionPtr active;
    QTcpServer* server;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_server == nullptr && !m_starting){
            //  Never started, or already closed.
            return;
        }
        m_state.store(State::DESTRUCTING, std::memory_order_release);
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        active = m_active;
        m_active = ConnectionPtr();
    }
    if (active){
        //  Close the connection (inert from now on; its object outlives
        //  the server) and cancel its in-flight sends, so their callers
        //  return promptly with the best-effort byte count.
        std::static_pointer_cast<ServerSocket2Connection_Qt>(active)->close_for_teardown();
    }

    //  Wait for an in-flight "start()" to finish so we destroy the
    //  server it creates.
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [this]{
            return !m_starting;
        });
        server = m_server;
        m_server = nullptr;
    }
    if (server == nullptr){
        //  "start()" failed or was never reached: nothing to destroy.
        std::lock_guard<Mutex> lg(m_lock);
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
        return;
    }

    //  Destroy the server on its event thread. This blocks until the
    //  destruction has happened.
    GlobalThreadPools::qt_event_threadpool().remove_object(server);

    std::lock_guard<Mutex> lg(m_lock);
    m_state.store(State::NOT_RUNNING, std::memory_order_release);
}

void ServerSocket2_Qt::on_new_connection(){
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

        auto typed_conn = std::make_shared<ServerSocket2Connection_Qt>(*this, sock);
        ConnectionPtr conn;
        bool rejected;
        {
            std::lock_guard<Mutex> lg(m_lock);
            State state = m_state.load(std::memory_order_relaxed);
            if (state == State::CONNECTED){
                //  A connection is already active: reject this client.
                rejected = true;
            }else if (state != State::LISTENING){
                //  The server is shutting down.
                rejected = false;
            }else{
                conn = typed_conn;
                m_active = conn;
                m_state.store(State::CONNECTED, std::memory_order_release);
                rejected = false;
            }
        }

        if (!conn){
            if (rejected){
                m_listeners.run_method(&Listener::on_connect_rejected);
            }
            sock->abort();
            //  The socket is never used again: delete it, so a peer that
            //  repeatedly connects (or keeps trying while another
            //  client is connected) cannot accumulate one QTcpSocket per
            //  attempt - they would otherwise stay children of the
            //  server until the whole server is destroyed.
            sock->deleteLater();
            continue;
        }

        QObject::connect(
            sock, &QAbstractSocket::readyRead,
            sock, [this, conn, typed_conn]{
                //  readyRead only fires once per delivery, and more than
                //  one buffer's worth of data may be waiting, so read
                //  until the socket has no more data available.
                while (true){
                    QTcpSocket* s = typed_conn->socket();
                    qint64 available = s->bytesAvailable();
                    if (available <= 0){
                        break;
                    }

                    constexpr size_t BUFFER_SIZE = 4096;
                    char buffer[BUFFER_SIZE];
                    qint64 bytes = s->read(buffer, std::min<qint64>(BUFFER_SIZE, available));
                    if (bytes <= 0){
                        break;
                    }

                    //  The connection may have been closed while we were
                    //  reading: only dispatch the data if we still are
                    //  the active connection.
                    bool still_active;
                    {
                        std::lock_guard<Mutex> lg(m_lock);
                        still_active = (m_active.get() == conn.get());
                    }
                    if (!still_active){
                        break;
                    }
                    m_listeners.run_method(&Listener::on_receive_data, conn, buffer, bytes);
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

        m_listeners.run_method(&Listener::on_connect, conn);
    }
}

void ServerSocket2_Qt::connection_lost(QTcpSocket* sock, const std::string& error_message){
    std::shared_ptr<ServerSocket2Connection_Qt> conn;
    bool report;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_active &&
            std::static_pointer_cast<ServerSocket2Connection_Qt>(m_active)->socket() == sock
        ){
            //  The first callback for the active connection wins;
            //  duplicates for the same socket return without reporting.
            conn = std::static_pointer_cast<ServerSocket2Connection_Qt>(m_active);
            m_active = ConnectionPtr();
            if (m_state.load(std::memory_order_relaxed) == State::CONNECTED){
                m_state.store(State::LISTENING, std::memory_order_release);
            }
            report = !conn->close_requested();
        }else{
            report = false;
        }
    }

    if (conn){
        //  The connection is dead from the application's point of view,
        //  even though the object outlives this (and the server).
        conn->mark_closed();
        if (report){
            m_listeners.run_method(&Listener::on_disconnect, conn, error_message);
        }
    }

    //  The socket is no longer tracked (m_active was nulled above, and
    //  the first callback for a socket wins), so it can be released:
    //  schedule its deletion on the event thread (this runs on it). A
    //  duplicate signal for the same socket returned above and does not
    //  schedule a second deletion; deleteLater() is idempotent either
    //  way.
    sock->deleteLater();
}

void ServerSocket2_Qt::on_close_connection(){
    QTcpSocket* sock;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_active != ConnectionPtr()){
            sock = std::static_pointer_cast<ServerSocket2Connection_Qt>(m_active)->socket();
            m_active = ConnectionPtr();
            if (m_state.load(std::memory_order_relaxed) != State::DESTRUCTING){
                //  The server keeps running, so it goes back to
                //  LISTENING. The destructor's close_and_wait() may have
                //  set DESTRUCTING: leave the state for it.
                m_state.store(State::LISTENING, std::memory_order_release);
            }
        }else{
            //  The connection was already gone (peer closed, or the
            //  destructor tore it down first): nothing to abort.
            sock = nullptr;
        }
    }
    if (sock != nullptr){
        sock->abort();
        //  The socket is no longer tracked (m_active was nulled above):
        //  schedule its deletion on the event thread. The
        //  disconnected/errorOccurred signals that abort() may still emit
        //  go through connection_lost(), which returns early because
        //  m_active is already null, so the socket is not scheduled
        //  twice.
        sock->deleteLater();
    }
}

void ServerSocket2_Qt::on_send(QSharedPointer<ServerSocket2Connection_Qt::SendData> send_data){
    ServerSocket2Connection_Qt* conn = send_data->conn.get();
    QTcpSocket* sock;
    {
        std::lock_guard<Mutex> lg(m_lock);
        //  The connection this send was queued for must still be the
        //  active one: a send for a closed (or replaced) connection must
        //  not reach into a later connection's socket.
        if (m_state.load(std::memory_order_relaxed) != State::CONNECTED
            || m_active.get() != conn
        ){
            sock = nullptr;
        }else{
            sock = conn->socket();
        }
    }

    {
        std::lock_guard<Mutex> lg(send_data->lock);
        if (send_data->cancelled){
            //  "close()"/"close_for_teardown()" already cancelled this
            //  send and woken its caller; finish without writing.
            return;
        }
    }

    size_t bytes_sent = 0;
    if (sock != nullptr){
        const char* ptr = send_data->packet.data();
        size_t remaining = send_data->packet.size();
        //  The deadline keeps a client that never reads from pinning
        //  this event thread forever (which would also block
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

    //  The queued call keeps the connection alive until it runs, even if
    //  the server (and the listeners) dropped their references.
    conn->finish_send(send_data, bytes_sent);
}

std::unique_ptr<QTcpServer> ServerSocket2_Qt::make_server(const std::string& address, uint16_t port){
    auto server = std::make_unique<QTcpServer>();
    if (!server->listen(QHostAddress(QString::fromStdString(address)), port)){
        std::lock_guard<Mutex> lg(m_lock);
        m_listen_error = server->errorString().toStdString();
    }

    //  Auto-connection, on purpose: when the caller is the event thread
    //  (a listener callback calling "send()"/"close()"), the handler runs
    //  inline inside the emit, so the call cannot deadlock waiting for
    //  its own queued event; the shell's lock is recursive for that
    //  re-entry. From another thread the call is queued. "send()" and
    //  "close()" hold the shell's lock through the emit: the teardown
    //  takes that same lock before it destroys the server, so the server
    //  is alive for the emit, and any drain that runs later still finds
    //  the send in the list and wakes its wait.
    QObject::connect(
        this, &ServerSocket2_Qt::internal_send,
        server.get(), [this](QSharedPointer<ServerSocket2Connection_Qt::SendData> send_data){
            on_send(send_data);
        }
    );
    QObject::connect(
        this, &ServerSocket2_Qt::internal_close_connection,
        server.get(), [this]{
            on_close_connection();
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
