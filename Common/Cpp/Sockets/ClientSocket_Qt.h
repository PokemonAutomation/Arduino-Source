/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_Qt_H
#define PokemonAutomation_ClientSocket_Qt_H

#include <iostream>
#include <QTcpSocket>
//#include "Common/Cpp/Concurrency/SpinPause.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Qt/QtThreadPool.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "AbstractClientSocket.h"

//using std::cout;
//using std::endl;

namespace PokemonAutomation{

class ThreadPool;



class ClientSocket_Qt final : public QObject, public AbstractClientSocket{
    Q_OBJECT

    struct SendData{
        const void* data;
        size_t total_bytes;
        size_t bytes_sent;
        Mutex lock;
        std::condition_variable cv;
    };


public:
    ClientSocket_Qt(ThreadPool&)
        : m_socket(
            static_cast<QTcpSocket*>(
                GlobalThreadPools::qt_event_threadpool().add_object(
                    [this]{ return make_socket(); }
                )
            )
        )
    {
//        cout << "ClientSocket_Qt()" << endl;
//        start();

//        cout << "ClientSocket_Qt() - waiting" << endl;
//        std::unique_lock<Mutex> lg(m_lock);
//        m_cv.wait(lg, [this]{ return m_socket != nullptr; });
    }

    virtual ~ClientSocket_Qt(){
//        cout << "~ClientSocket_Qt()" << endl;
        close();
    }
    virtual void close() noexcept override{
        GlobalThreadPools::qt_event_threadpool().remove_object(m_socket);
//        cout << "close()" << endl;
        m_state.store(State::DESTRUCTING, std::memory_order_release);
//        quit();
//        wait();
    }

    virtual void connect(const std::string& address, uint16_t port) override{
//        cout << "connect()" << endl;
        emit internal_connect(address, port);
    }

    virtual size_t send(const void* data, size_t bytes) override{
//        cout << "send() - start: " << std::string((const char*)data, bytes) << endl;
        emit internal_send(std::string((const char*)data, bytes));
        return bytes;

#if 0
        SendData send_data;
        send_data.data = data;
        send_data.total_bytes = bytes;
        send_data.bytes_sent = 0;

        emit send(&send_data);

        std::unique_lock<Mutex> lg(send_data.lock);
        send_data.cv.wait(lg, [&]{
            return send_data.data == nullptr || m_socket == nullptr;
        });

//        cout << "send() - end: " << std::string((const char*)data, bytes) << endl;
        return send_data.bytes_sent;
#endif
    }


signals:
    void internal_connect(const std::string& address, uint16_t port);
    void internal_send(std::string packet);

private:
    std::unique_ptr<QTcpSocket> make_socket(){
        auto socket = std::make_unique<QTcpSocket>();

        QObject::connect(
            socket.get(), &QTcpSocket::connected,
            socket.get(), [this]{
//                cout << "connected()" << endl;
                m_state.store(State::CONNECTED, std::memory_order_release);
                m_listeners.run_method(&Listener::on_connect_finished, "");
            }
        );
        QObject::connect(
            socket.get(), &QTcpSocket::disconnected,
            socket.get(), [this]{
                std::cout << "QTcpSocket::disconnected()" << std::endl;
                m_state.store(State::DESTRUCTING, std::memory_order_release);
//                quit();
            }
        );
        QObject::connect(
            socket.get(), &QTcpSocket::errorOccurred,
            socket.get(), [this](QAbstractSocket::SocketError error){
                std::cout << "QTcpSocket::errorOccurred(): error = " << (int)error <<  std::endl;
                m_state.store(State::DESTRUCTING, std::memory_order_release);
//                quit();
            }
        );
        QObject::connect(
            socket.get(), &QTcpSocket::readyRead,
            socket.get(), [this]{
//                cout << "readyRead()" << endl;
                constexpr size_t BUFFER_SIZE = 4096;
                char buffer[BUFFER_SIZE];
                qint64 bytes = m_socket->read(buffer, BUFFER_SIZE);
                if (bytes > 0){
//                    cout << "Received: " << std::string(buffer, bytes) << endl;
                    m_listeners.run_method(&Listener::on_receive_data, buffer, bytes);
                }
            }
        );
        QObject::connect(
            this, &ClientSocket_Qt::internal_connect,
            socket.get(), [this](const std::string& address, uint16_t port){
                m_state.store(State::CONNECTING, std::memory_order_release);
                m_socket->connectToHost(QHostAddress(QString::fromStdString(address)), port);
            }
        );
        QObject::connect(
            this, &ClientSocket_Qt::internal_send,
            socket.get(), [this](std::string packet){
//                cout << "internal_send() - enter: " << packet.data() << endl;

                size_t bytes = packet.size();

                const char* ptr = (const char*)packet.data();
                while (bytes > 0 && m_socket->state() == QAbstractSocket::ConnectedState){
                    qint64 current_sent = m_socket->write(ptr, bytes);
                    if (current_sent <= 0){
                        break;
                    }
                    ptr += current_sent;
                    bytes -= current_sent;
                }

//                m_socket->flush();

//                cout << "internal_send() - exit " << endl;
            }
        );

        return socket;
    }

private:
    Mutex m_lock;
    ConditionVariable m_cv;
    QTcpSocket* m_socket;
};





}
#endif
