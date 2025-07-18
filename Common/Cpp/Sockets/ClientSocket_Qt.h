/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_Qt_H
#define PokemonAutomation_ClientSocket_Qt_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <QThread>
#include <QTcpSocket>
//#include "Common/Cpp/Concurrency/SpinPause.h"
#include "AbstractClientSocket.h"

//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class ClientSocket_Qt final : public QThread, public AbstractClientSocket{
    Q_OBJECT

    struct SendData{
        const void* data;
        size_t total_bytes;
        size_t bytes_sent;
        std::mutex lock;
        std::condition_variable cv;
    };


public:
    ClientSocket_Qt()
        : m_socket(nullptr)
    {
//        cout << "ClientSocket_Qt()" << endl;
        start();

//        cout << "ClientSocket_Qt() - waiting" << endl;
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.wait(lg, [this]{ return m_socket != nullptr; });
    }

    virtual ~ClientSocket_Qt(){
//        cout << "~ClientSocket_Qt()" << endl;
        close();
    }
    virtual void close() noexcept override{
//        cout << "close()" << endl;
        m_state.store(State::DESTRUCTING, std::memory_order_release);
        quit();
        wait();
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

        std::unique_lock<std::mutex> lg(send_data.lock);
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
    virtual void run() override{
        QTcpSocket socket;

        QThread::connect(
            &socket, &QTcpSocket::connected,
            &socket, [this]{
//                cout << "connected()" << endl;
                m_state.store(State::CONNECTED, std::memory_order_release);
                m_listeners.run_method_unique(&Listener::on_connect_finished, "");
            }
        );
        QThread::connect(
            &socket, &QTcpSocket::disconnected,
            &socket, [this]{
                std::cout << "QTcpSocket::disconnected()" << std::endl;
                m_state.store(State::DESTRUCTING, std::memory_order_release);
                quit();
            }
        );
        QThread::connect(
            &socket, &QTcpSocket::errorOccurred,
            &socket, [this](QAbstractSocket::SocketError error){
                std::cout << "QTcpSocket::errorOccurred(): error = " << (int)error <<  std::endl;
                m_state.store(State::DESTRUCTING, std::memory_order_release);
                quit();
            }
        );
        QThread::connect(
            &socket, &QTcpSocket::readyRead,
            &socket, [this]{
//                cout << "readyRead()" << endl;
                constexpr size_t BUFFER_SIZE = 4096;
                char buffer[BUFFER_SIZE];
                qint64 bytes = m_socket->read(buffer, BUFFER_SIZE);
                if (bytes > 0){
//                    cout << "Received: " << std::string(buffer, bytes) << endl;
                    m_listeners.run_method_unique(&Listener::on_receive_data, buffer, bytes);
                }
            }
        );
        QThread::connect(
            this, &ClientSocket_Qt::internal_connect,
            &socket, [this](const std::string& address, uint16_t port){
                m_state.store(State::CONNECTING, std::memory_order_release);
                m_socket->connectToHost(QHostAddress(QString::fromStdString(address)), port);
            }
        );
        QThread::connect(
            this, &ClientSocket_Qt::internal_send,
            &socket, [this](std::string packet){
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
#if 0
        QThread::connect(
            this, &ClientSocket_Qt::send,
            &socket, [this](void* params){
//                cout << "internal_send() - enter " << endl;

                SendData& data = *(SendData*)params;
                size_t sent = 0;

                size_t bytes = data.total_bytes;

                const char* ptr = (const char*)data.data;
                while (bytes > 0 && m_socket->state() == QAbstractSocket::ConnectedState){
                    qint64 current_sent = m_socket->write((const char*)ptr, bytes);
                    if (current_sent <= 0){
                        break;
                    }
                    sent += current_sent;
                    ptr += current_sent;
                    bytes -= current_sent;
                }

                m_socket->flush();

                std::lock_guard<std::mutex> lg(data.lock);
                data.data = nullptr;
                data.bytes_sent = sent;
                data.cv.notify_all();

//                cout << "internal_send() - exit " << endl;
            }
        );
#endif


        {
            std::lock_guard<std::mutex> lg(m_lock);
            if (this->state() == State::DESTRUCTING){
                return;
            }
            m_socket = &socket;
        }
        m_cv.notify_all();

        exec();

        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_socket = nullptr;
        }
        m_cv.notify_all();
    }

private:
    std::mutex m_lock;
    std::condition_variable m_cv;
    QTcpSocket* m_socket;
};





}
#endif
