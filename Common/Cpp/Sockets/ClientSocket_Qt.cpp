/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <algorithm>
#include <QSharedPointer>
#include <QTimer>
#include <QTcpSocket>
#include <QMetaObject>
#include "Common/Qt/QtThreadPool.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "ClientSocket_Qt.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void ClientSocket_Qt::close_and_wait() noexcept{
    QTcpSocket* socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_socket == nullptr){
            //  Never connected, or already closed.
            return;
        }
        m_state.store(State::DESTRUCTING, std::memory_order_release);
        m_success_pending = false;
        socket = m_socket;
        m_socket = nullptr;
    }

    //  Cancel the in-flight "send()" calls so their callers return
    //  promptly with the best-effort byte count, instead of waiting for
    //  the event thread to finish the writes.
    cancel_inflight_sends();

    //  Destroy the socket on its event thread. This blocks until the
    //  destruction has happened. A queued "send()" call still runs before
    //  the destruction (the event loop is FIFO), but it is already
    //  cancelled and finishes without writing.
    GlobalThreadPools::qt_event_threadpool().remove_object(socket);

    std::lock_guard<Mutex> lg(m_lock);
    m_state.store(State::NOT_RUNNING, std::memory_order_release);
}
void ClientSocket_Qt::close() noexcept{
    QTcpSocket* socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_socket == nullptr){
            return;
        }
        m_state.store(State::DESTRUCTING, std::memory_order_release);
        m_success_pending = false;
        socket = m_socket;
    }

    //  Immediately break the connection. QTcpSocket::abort() must run on the
    //  socket's event thread, but "close()" may be called from any thread
    //  (including a listener callback, which runs on that event thread), so
    //  post the abort to the socket's event thread instead of calling it here
    //  - a cross-thread call would trip Qt's QSocketNotifier thread check.
    //  This stays non-blocking: close() returns immediately, and the socket is
    //  aborted as soon as the event thread reaches the queued call. The
    //  resulting "disconnected"/"errorOccurred" signals see State::DESTRUCTING
    //  and are no-ops. The destructor's close_and_wait() destroys the socket
    //  and returns it to NOT_RUNNING.
    cancel_inflight_sends();
    QMetaObject::invokeMethod(
        socket,
        [socket]{ socket->abort(); },
        Qt::QueuedConnection
    );
}
void ClientSocket_Qt::connect(const std::string& address, uint16_t port){
    //  Complete any in-progress connection or a requested-but-unfinished
    //  close() before starting a new one. close() is non-blocking (it leaves
    //  the socket DESTRUCTING), so reconnecting on the same socket destroys the
    //  old QTcpSocket here (on its event thread) to get back to NOT_RUNNING.
    //  No-op if the socket is already NOT_RUNNING.
    close_and_wait();

    QTcpSocket* old_socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_state.load(std::memory_order_relaxed) != State::NOT_RUNNING){
            return;
        }
        old_socket = m_socket;
        m_socket = nullptr;
    }
    if (old_socket != nullptr){
        //  Cancel the in-flight sends before destroying the socket: a
        //  call still queued is discarded with the socket, and its
        //  caller must not be left waiting for a "done" that never
        //  comes.
        cancel_inflight_sends();

        //  Destroy the socket from the previous connection. This blocks
        //  until the destruction has happened.
        GlobalThreadPools::qt_event_threadpool().remove_object(old_socket);
    }

    //  A new QTcpSocket for every connection. A socket whose connection
    //  has ended cannot be reconnected.
    QTcpSocket* socket = static_cast<QTcpSocket*>(
        GlobalThreadPools::qt_event_threadpool().add_object(
            [this]{ return make_socket(); }
        )
    );
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_socket = socket;
        m_success_pending = false;
        m_state.store(State::CONNECTING, std::memory_order_release);
    }

    emit internal_connect(address, port);
}

size_t ClientSocket_Qt::send(const void* data, size_t bytes, std::chrono::milliseconds timeout){
    QTcpSocket* socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket = m_socket;
    }
    if (socket == nullptr || state() != State::CONNECTED){
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

    //  Register the send atomically with respect to "close()"/"connect()":
    //  both detach "m_socket" under this same lock before they cancel the
    //  in-flight sends. The packet copy above can take a while for large
    //  payloads, so without this re-check a send could register *after*
    //  "close()" already cancelled an (empty) list and queued the socket's
    //  destruction: its "internal_send" call would then be posted after the
    //  destruction event, Qt would silently discard it, and the wait below
    //  (unbounded for the infinite default timeout) would never end. With
    //  the re-check, a send either registers before the cancel (and gets
    //  cancelled, waking the wait below) or sees the detached socket and
    //  returns without queueing anything.
    bool registered;
    {
        std::lock_guard<Mutex> lg(m_lock);
        registered = (m_socket != nullptr);
        if (registered){
            m_inflight_sends.push_back(send_data);
        }
    }
    if (!registered){
        return 0;
    }

    emit internal_send(send_data);

    //  Wait for the socket's event thread to finish writing the packet,
    //  or for "close()"/"connect()" to cancel the send. A finite
    //  "timeout" waits for the deadline plus 1 s of queueing margin: the
    //  write loop stops at the deadline, so anything beyond that is just
    //  the queued call waiting to start.
    //
    //  The infinite default waits without a time bound. The registration
    //  above is atomic with the "close()"/"connect()" detach, so "done" is
    //  set by either the event-thread write loop (which stops at the
    //  deadline, on a socket error, or when Qt's userspace buffer accepts
    //  the packet; it runs before the queued socket destruction, since the
    //  event loop is FIFO) or by "close()"/"connect()" cancelling the
    //  registered send - even when Qt discards the queued write loop with
    //  the socket - so the wait always ends. The shared ownership keeps
    //  the packet alive until the queued call runs (or is discarded with
    //  the socket).
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

void ClientSocket_Qt::cancel_inflight_sends(){
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

void ClientSocket_Qt::connection_failed(const std::string& error_message){
    bool as_connect_failure;
    {
        std::lock_guard<Mutex> lg(m_lock);
        State state = m_state.load(std::memory_order_relaxed);
        if (state == State::CONNECTING){
            as_connect_failure = true;
        }else if (state == State::CONNECTED){
            as_connect_failure = m_success_pending;
        }else{
            return;
        }
        m_success_pending = false;
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
    }

    if (as_connect_failure){
        m_listeners.run_method(&Listener::on_connect_finished, error_message);
    }else{
        m_listeners.run_method(&Listener::on_disconnect, error_message);
    }
}


std::unique_ptr<QTcpSocket> ClientSocket_Qt::make_socket(){
    auto socket = std::make_unique<QTcpSocket>();

    QObject::connect(
        socket.get(), &QTcpSocket::connected,
        socket.get(), [this, sock = socket.get()]{
            bool pending_success;
            {
                std::lock_guard<Mutex> lg(m_lock);
                pending_success =
                    (m_state.load(std::memory_order_relaxed) == State::CONNECTING);
                if (pending_success){
                    m_state.store(State::CONNECTED, std::memory_order_release);
                    m_success_pending = true;
                }
            }
            if (!pending_success){
                return;
            }

            //  On some platforms Qt emits "connected" optimistically,
            //  before the connection attempt is definitively settled - a
            //  refused connection emits "connected" and then immediately
            //  "errorOccurred". So only report success once a short grace
            //  period has passed without an error.
            QTimer::singleShot(
                50, sock,
                [this]{
                    bool fire;
                    {
                        std::lock_guard<Mutex> lg(m_lock);
                        fire = m_success_pending;
                        fire &= m_state.load(std::memory_order_relaxed) == State::CONNECTED;
                        if (fire){
                            m_success_pending = false;
                        }
                    }
                    if (fire){
                        m_listeners.run_method(&Listener::on_connect_finished, "");
                    }
                }
            );
        }
    );
    QObject::connect(
        socket.get(), &QTcpSocket::disconnected,
        socket.get(), [this, sock = socket.get()]{
            //  If a socket error caused this disconnect, "errorOccurred"
            //  has already reported it and changed the state, so this
            //  only produces a callback for a clean close.
            std::string message;
            if (sock->error() != QAbstractSocket::SocketError::UnknownSocketError){
                message = sock->errorString().toStdString();
            }else{
                message = "Connection closed by server.";
            }
            connection_failed(message);
        }
    );
    QObject::connect(
        socket.get(), &QTcpSocket::errorOccurred,
        socket.get(), [this, sock = socket.get()](QAbstractSocket::SocketError){
            connection_failed(sock->errorString().toStdString());
        }
    );
    QObject::connect(
        socket.get(), &QTcpSocket::readyRead,
        socket.get(), [this, sock = socket.get()]{
            //  readyRead only fires once per delivery, and more than one
            //  buffer's worth of data may be waiting, so read until the
            //  socket has no more data available.
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
        this, &ClientSocket_Qt::internal_connect,
        socket.get(), [this, sock = socket.get()](const std::string& address, uint16_t port){
            std::lock_guard<Mutex> lg(m_lock);
            if (m_state.load(std::memory_order_relaxed) == State::DESTRUCTING){
                return;
            }
            sock->connectToHost(QHostAddress(QString::fromStdString(address)), port);
        }
    );
    QObject::connect(
        this, &ClientSocket_Qt::internal_send,
        socket.get(), [this, sock = socket.get()](QSharedPointer<SendData> send_data){
            {
                std::lock_guard<Mutex> lg(send_data->lock);
                if (send_data->cancelled){
                    //  "close()"/"connect()" already cancelled this send
                    //  and woken its caller; finish without writing.
                    return;
                }
            }

            size_t bytes_sent = 0;
            {
                const char* ptr = send_data->packet.data();
                size_t remaining = send_data->packet.size();
                //  The deadline keeps a peer that never reads from pinning
                //  this event thread forever (which would also block
                //  "close()" on it).
                while (
                    remaining > 0
                    && sock->state() == QAbstractSocket::ConnectedState
                    && std::chrono::steady_clock::now() < send_data->deadline
                ){
                    {
                        std::lock_guard<Mutex> lg(send_data->lock);
                        if (send_data->cancelled){
                            //  "close()" cancelled the send mid-write; its
                            //  caller has already been woken with the
                            //  count written so far.
                            break;
                        }
                    }
                    qint64 current_sent = sock->write(ptr, remaining);
                    if (current_sent <= 0){
                        //  The kernel's send buffer is full. Wait for
                        //  some of it to drain, then try again.
                        sock->waitForBytesWritten(10);
                        continue;
                    }
                    ptr += current_sent;
                    remaining -= (size_t)current_sent;
                    //  Track the running count, so a "close()" that
                    //  cancels this send mid-write reports the bytes
                    //  written so far.
                    {
                        std::lock_guard<Mutex> lg(send_data->lock);
                        send_data->bytes_sent = send_data->packet.size() - remaining;
                    }
                }
                bytes_sent = send_data->packet.size() - remaining;
            }

            {
                std::lock_guard<Mutex> lg(m_lock);
                //  "close()"/"connect()" may already have claimed this
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
    );

    return socket;
}



}
