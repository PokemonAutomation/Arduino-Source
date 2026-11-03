/*  Server Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_Qt_H
#define PokemonAutomation_ServerSocket_Qt_H

#include <chrono>
#include <string>
#include <atomic>
#include <vector>
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
        //  The event-thread write loop stops at this deadline, so a send
        //  that can never complete (a client that never reads) cannot pin
        //  the server's event thread forever and block "close()". "max()"
        //  means no deadline (the infinite default).
        std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
        //  The running byte count: the write loop updates it as it goes,
        //  so "close()"/"close_connection()" cancelling the send mid-write
        //  can report the bytes written so far (best effort).
        size_t bytes_sent = 0;
        //  Set by "close()"/"close_connection()" when they cancel the
        //  send; the queued write loop then finishes without writing.
        bool cancelled = false;
        //  Set when the event-thread write loop finishes or when
        //  "close()"/"close_connection()" cancel the send.
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

    //  Closes the active connection, if any, and keeps listening. It
    //  cancels the in-flight "send()" calls: their callers return
    //  promptly with the number of bytes actually written so far (best
    //  effort). It is safe to call from within a listener callback.
    virtual void close_connection() override;

    //  Stops the server, destroys the QTcpServer (and its connection
    //  socket), and returns the socket to State::NOT_RUNNING so "start()"
    //  can be called again. This blocks until the QTcpServer has been
    //  destroyed, so any in-flight listener callback is finished by the
    //  time it returns. It is safe to call multiple times.
    //
    //  It cancels the in-flight "send()" calls the same way
    //  "close_connection()" does: their callers return promptly with the
    //  number of bytes actually written so far (best effort).
    //
    //  This must not be called from within a listener callback, since the
    //  callbacks run on the QTcpServer's event thread and this destroys the
    //  server that is running them.
    virtual void close() noexcept override;

    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    ) override;


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

    //  Cancels the in-flight "send()" calls: marks them done so their
    //  callers return promptly with the bytes written so far (best
    //  effort). The queued write loops finish without writing. The
    //  shared ownership keeps each packet alive until its queued call
    //  runs (or is discarded with the server).
    void cancel_inflight_sends();

    std::unique_ptr<QTcpServer> make_server(const std::string& address, uint16_t port);

private:
    Mutex m_lock;
    ConditionVariable m_cv;

    QTcpServer* m_server = nullptr;
    QTcpSocket* m_socket = nullptr;
    QThread* m_event_thread = nullptr;
    bool m_close_requested = false;
    bool m_starting = false;

    //  The "send()" calls that are queued on the event thread but not
    //  finished yet, so that "close()"/"close_connection()" can cancel
    //  them (and wake their callers) before the socket is destroyed. The
    //  write loops remove their entry when they finish. Protected by
    //  m_lock.
    std::vector<QSharedPointer<SendData>> m_inflight_sends;

    //  Written on the event thread by "make_server()", read after
    //  "add_object()" has returned.
    std::string m_listen_error;
};



}
#endif