/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_Qt_H
#define PokemonAutomation_ClientSocket_Qt_H

#include <string>
#include <atomic>
#include <condition_variable>
#include <QObject>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "AbstractClientSocket.h"

class QTcpSocket;

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
    virtual void close() noexcept override;

    virtual void connect(const std::string& address, uint16_t port) override;

    virtual size_t send(const void* data, size_t bytes) override;


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
    void connection_failed(const std::string& error_message);

    std::unique_ptr<QTcpSocket> make_socket();


private:
    Mutex m_lock;
    ConditionVariable m_cv;

    QTcpSocket* m_socket = nullptr;
    bool m_success_pending = false;
    std::atomic<int> m_sends_in_flight{0};
};





}
#endif
