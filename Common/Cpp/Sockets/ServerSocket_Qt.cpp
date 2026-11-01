/*  Server Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
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
        m_starting = true;
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
    }
    catch (...){
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_starting = false;
            m_cv.notify_all();
        }
        m_listeners.run_method(&Listener::on_listen_finished, "Failed to start the listener.");
        throw;
    }
    bool listening;
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_starting = false;
        m_server = server;
        if (m_state.load(std::memory_order_relaxed) == State::DESTRUCTING){
            //  "close()" is already in progress: it will destroy the
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
void ServerSocket_Qt::close() noexcept{
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

    //  Wait for an in-flight "start()" to finish so we destroy the
    //  server it creates, and for in-flight "send()" calls to finish so
    //  we don't destroy the server out from under them.
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [this]{
            return !m_starting
                && m_sends_in_flight.load(std::memory_order_acquire) == 0;
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

size_t ServerSocket_Qt::send(const void* data, size_t bytes){
    QTcpSocket* socket;
    {
        std::lock_guard<Mutex> lg(m_lock);
        socket =
            (m_state.load(std::memory_order_relaxed) == State::CONNECTED) ? m_socket : nullptr;
    }
    if (socket == nullptr){
        return 0;
    }

    QSharedPointer<SendData> send_data = QSharedPointer<SendData>::create();
    send_data->packet.assign((const char*)data, bytes);

    emit internal_send(send_data);

    //  Wait for the server's event thread to finish writing the packet.
    //  The timeout covers the case where "close()" destroys the server
    //  before the queued call ever runs, in which case it will never
    //  complete on its own; the shared ownership keeps the packet alive
    //  until the queued call runs (or is discarded with the server).
    {
        std::unique_lock<Mutex> lg(send_data->lock);
        send_data->cv.wait_for(
            lg,
            std::chrono::seconds(5),
            [&send_data]{ return send_data->done; }
        );
    }

    return send_data->bytes_sent;
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
        //  The connection socket lives with the server, so "close()"
        //  destroys it on the event thread.
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
}

void ServerSocket_Qt::on_close_connection(QSharedPointer<CloseCompletion> completion){
    QTcpSocket* sock;
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_socket != nullptr){
            m_close_requested = true;
            m_state.store(State::LISTENING, std::memory_order_release);
            sock = m_socket;
            m_socket = nullptr;
        }else{
            sock = nullptr;
        }
    }
    if (sock != nullptr){
        sock->abort();
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
        m_sends_in_flight.fetch_add(1, std::memory_order_acq_rel);
        m_cv.notify_all();
        sock = m_socket;
    }

    size_t bytes_sent = 0;
    if (sock != nullptr){
        const char* ptr = send_data->packet.data();
        size_t remaining = send_data->packet.size();
        while (remaining > 0 && sock->state() == QAbstractSocket::ConnectedState){
            qint64 current_sent = sock->write(ptr, remaining);
            if (current_sent <= 0){
                //  The kernel's send buffer is full. Wait for some of it
                //  to drain, then try again.
                sock->waitForBytesWritten(10);
                continue;
            }
            ptr += current_sent;
            remaining -= (size_t)current_sent;
        }
        bytes_sent = send_data->packet.size() - remaining;
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        m_sends_in_flight.fetch_sub(1, std::memory_order_acq_rel);
        m_cv.notify_all();
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
