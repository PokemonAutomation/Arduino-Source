/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_Qt_H
#define PokemonAutomation_ClientSocket_Qt_H

#include <mutex>
#include <condition_variable>
#include <QThread>
#include <QTcpSocket>
//#include "Common/Cpp/Concurrency/SpinPause.h"
#include "AbstractClientSocket.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class ClientSocket_Qt final : public QThread, public AbstractClientSocket{
    Q_OBJECT

    struct SendData{
        const void* data;
        size_t bytes;
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

    virtual size_t blocking_send(const void* data, size_t bytes) override{
//        cout << "blocking_send() - start: " << std::string((const char*)data, bytes) << endl;

        SendData send_data;
        send_data.data = data;
        send_data.bytes = bytes;

        emit send(&send_data);

        std::unique_lock<std::mutex> lg(send_data.lock);
        send_data.cv.wait(lg, [&]{
            return send_data.data == nullptr || m_socket == nullptr;
        });

//        cout << "blocking_send() - end: " << std::string((const char*)data, bytes) << endl;
        return send_data.bytes;
    }


signals:
    void internal_connect(const std::string& address, uint16_t port);
    void send(void* data);

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
            this, &ClientSocket_Qt::send,
            &socket, [this](void* params){
//                cout << "internal_send() - enter " << endl;

                SendData& data = *(SendData*)params;
                size_t sent = 0;

                size_t bytes = data.bytes;

                const char* ptr = (const char*)data.data;
                while (bytes > 0 && m_socket->state() == QAbstractSocket::ConnectedState){
                    qint64 current_sent = m_socket->write((const char*)ptr, bytes);
                    if (current_sent <= 0){
                        break;
                    }
                    sent += current_sent;
                    ptr += current_sent;
                    bytes -= current_sent;
                    data.bytes = sent;
                }

                m_socket->flush();

                std::lock_guard<std::mutex> lg(data.lock);
                data.data = nullptr;
                data.bytes = sent;
                data.cv.notify_all();

//                cout << "internal_send() - exit " << endl;
            }
        );


        {
            std::lock_guard<std::mutex> lg(m_lock);
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
