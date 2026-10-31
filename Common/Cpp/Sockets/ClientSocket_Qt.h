/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_Qt_H
#define PokemonAutomation_ClientSocket_Qt_H

#include <string>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <QTcpSocket>
#include <QTimer>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Qt/QtThreadPool.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "AbstractClientSocket.h"

namespace PokemonAutomation{


class ThreadPool;


class ClientSocket_Qt final : public QObject, public AbstractClientSocket{
    Q_OBJECT

    struct SendData{
        std::string packet;
        size_t bytes_sent = 0;
        bool done = false;

        Mutex lock;
        std::condition_variable cv;
    };


public:
    ClientSocket_Qt(ThreadPool&){}

    virtual ~ClientSocket_Qt(){
        close();
    }

    //  Stops the connection, destroys the QTcpSocket, and returns the socket
    //  to State::NOT_RUNNING so "connect()" can be called again. This blocks
    //  until the QTcpSocket has been destroyed, so any in-flight listener
    //  callback is finished by the time it returns. It is safe to call
    //  multiple times.
    //
    //  This must not be called from within a listener callback, since the
    //  callbacks run on the QTcpSocket's event thread and this destroys the
    //  socket that is running them.
    virtual void close() noexcept override{
        QTcpSocket* socket;
        {
            std::lock_guard<Mutex> lg(m_lock);
            if (m_socket == nullptr){
                //  Never connected, or already closed.
                return;
            }
            m_state.store(State::DESTRUCTING, std::memory_order_release);
            m_success_pending = false;
            m_cv.notify_all();
            socket = m_socket;
            m_socket = nullptr;
        }

        //  Wait for in-flight "send()" calls to finish so we don't destroy
        //  the socket out from under them.
        {
            std::unique_lock<Mutex> lg(m_lock);
            m_cv.wait(lg, [this]{
                return m_sends_in_flight.load(std::memory_order_acquire) == 0;
            });
        }

        //  Destroy the socket on its event thread. This blocks until the
        //  destruction has happened.
        GlobalThreadPools::qt_event_threadpool().remove_object(socket);

        std::lock_guard<Mutex> lg(m_lock);
        m_state.store(State::NOT_RUNNING, std::memory_order_release);
    }

    virtual void connect(const std::string& address, uint16_t port) override{
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

    virtual size_t send(const void* data, size_t bytes) override{
        QTcpSocket* socket;
        {
            std::lock_guard<Mutex> lg(m_lock);
            socket = m_socket;
        }
        if (socket == nullptr || state() != State::CONNECTED){
            return 0;
        }

        SendData send_data;
        send_data.packet.assign((const char*)data, bytes);

        emit internal_send(&send_data);

        //  Wait for the socket's event thread to finish writing the packet.
        //  The timeout covers the case where "close()" destroys the socket
        //  before the queued call ever runs, in which case it will never
        //  complete on its own.
        {
            std::unique_lock<Mutex> lg(send_data.lock);
            send_data.cv.wait_for(
                lg,
                std::chrono::seconds(5),
                [&send_data]{ return send_data.done; }
            );
        }

        return send_data.bytes_sent;
    }


signals:
    void internal_connect(const std::string& address, uint16_t port);
    void internal_send(SendData* send_data);


private:
    //  The connection attempt or the connection itself ended in a failure.
    //  This fires the appropriate callback and returns the socket to
    //  State::NOT_RUNNING so "connect()" can be called again. If the
    //  connection was only tentatively confirmed (see the "connected" slot
    //  below), it is still reported as a failed connect attempt. Only the
    //  first of the (errorOccurred, disconnected) signals for a given
    //  failure fires a callback.
    //  Runs on the QTcpSocket's event thread.
    void connection_failed(const std::string& error_message){
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

    std::unique_ptr<QTcpSocket> make_socket(){
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
            socket.get(), [this, sock = socket.get()](SendData* send_data){
                {
                    std::lock_guard<Mutex> lg(m_lock);
                    m_sends_in_flight.fetch_add(1, std::memory_order_acq_rel);
                    m_cv.notify_all();
                }

                size_t bytes_sent = 0;
                {
                    const char* ptr = send_data->packet.data();
                    size_t remaining = send_data->packet.size();
                    while (remaining > 0 && sock->state() == QAbstractSocket::ConnectedState){
                        qint64 current_sent = sock->write(ptr, remaining);
                        if (current_sent <= 0){
                            //  The kernel's send buffer is full. Wait for
                            //  some of it to drain, then try again.
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
        );

        return socket;
    }

private:
    Mutex m_lock;
    ConditionVariable m_cv;

    QTcpSocket* m_socket = nullptr;
    bool m_success_pending = false;
    std::atomic<int> m_sends_in_flight{0};
};





}
#endif
