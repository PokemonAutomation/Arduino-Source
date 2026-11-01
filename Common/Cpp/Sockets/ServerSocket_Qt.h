/*  Server Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_Qt_H
#define PokemonAutomation_ServerSocket_Qt_H

#include <string>
#include <atomic>
#include <QObject>
#include <QSharedPointer>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "AbstractServerSocket.h"

class QThread;
class QTcpServer;
class QTcpSocket;

namespace PokemonAutomation{


class ThreadPool;


class ServerSocket_Qt final : public QObject, public AbstractServerSocket{
    Q_OBJECT

    //  Carries shared ownership (the signals pass it by value) so that a call still
    //  queued when the caller times out, or when "close()" destroys the server and
    //  Qt discards the pending call, keeps the payload alive exactly as long as the
    //  event itself: the handler's copy outlives a timed-out caller, and a
    //  discarded event leaves the caller's copy as the last reference.
    struct SendData{
        std::string packet;
        size_t bytes_sent = 0;
        bool done = false;

        Mutex lock;
        ConditionVariable cv;
    };

    //  Synchronizes "close_connection()" with the QTcpServer's event thread when
    //  called from another thread. Lifetime as SendData.
    struct CloseCompletion{
        Mutex lock;
        ConditionVariable cv;
        bool done = false;
    };

public:
    ServerSocket_Qt(ThreadPool&){}

    virtual ~ServerSocket_Qt(){
        close();
    }

    //  Binds to "address" and listens on "port" on the Qt event thread. This
    //  blocks until the QTcpServer has been created on that thread.
    virtual void start(const std::string& address, uint16_t port) override;

    virtual void close_connection() override;

    //  Stops the server, destroys the QTcpServer (and its connection
    //  socket), and returns the socket to State::NOT_RUNNING so "start()"
    //  can be called again. This blocks until the QTcpServer has been
    //  destroyed, so any in-flight listener callback is finished by the
    //  time it returns. It is safe to call multiple times.
    //
    //  This must not be called from within a listener callback, since the
    //  callbacks run on the QTcpServer's event thread and this destroys the
    //  server that is running them.
    virtual void close() noexcept override;

    virtual size_t send(const void* data, size_t bytes) override;


signals:
    void internal_send(QSharedPointer<SendData> send_data);
    void internal_close_connection(QSharedPointer<CloseCompletion> completion);


private:
    //  Runs on the QTcpServer's event thread.
    void on_new_connection();

    //  The active connection ended, or was aborted by "close_connection()".
    //  Only the first of the (errorOccurred, disconnected) signals for a
    //  given connection fires a callback, and "on_disconnect" is not fired
    //  for a connection closed by "close_connection()" or "close()".
    //  Runs on the QTcpServer's event thread.
    void connection_lost(QTcpSocket* sock, const std::string& error_message);

    //  Runs on the QTcpServer's event thread.
    void on_close_connection(QSharedPointer<CloseCompletion> completion);

    //  Runs on the QTcpServer's event thread. The context object is the
    //  server, so the call still runs if the connection socket has already
    //  been aborted, and it just reports 0 bytes in that case.
    void on_send(QSharedPointer<SendData> send_data);

    std::unique_ptr<QTcpServer> make_server(const std::string& address, uint16_t port);

private:
    Mutex m_lock;
    ConditionVariable m_cv;

    QTcpServer* m_server = nullptr;
    QTcpSocket* m_socket = nullptr;
    QThread* m_event_thread = nullptr;
    bool m_close_requested = false;
    bool m_starting = false;
    std::atomic<int> m_sends_in_flight{0};

    //  Written on the event thread by "make_server()", read after
    //  "add_object()" has returned.
    std::string m_listen_error;
};



}
#endif