/*  Client Socket (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_Qt_H
#define PokemonAutomation_ClientSocket_Qt_H

#include <chrono>
#include <string>
#include <vector>
#include <QObject>
#include <QSharedPointer>
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
        //  The event-thread write loop stops at this deadline, so a send
        //  that can never complete (a peer that never reads) cannot pin
        //  the socket's event thread forever and block "close_and_wait()".
        //  "max()" means no deadline (the infinite default).
        std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
        //  The running byte count: the write loop updates it as it goes,
        //  so "close()" cancelling the send mid-write can report the
        //  bytes written so far (best effort).
        size_t bytes_sent = 0;
        //  Set by "close()"/"connect()" when they cancel the send; the
        //  queued write loop then finishes without writing.
        bool cancelled = false;
        //  Set when the event-thread write loop finishes or when
        //  "close()"/"connect()" cancel the send.
        bool done = false;

        Mutex lock;
        ConditionVariable cv;
    };


public:
    ClientSocket_Qt(ThreadPool&){}

    ~ClientSocket_Qt() override{
        close_and_wait();
    }

    //  Requests immediate teardown of the active connection, without waiting
    //  for the QTcpSocket's event thread to destroy it. Safe to call from
    //  within a listener callback (which runs on that event thread). It marks
    //  the socket DESTRUCTING and aborts the underlying QTcpSocket; the
    //  resulting disconnect signals are no-ops. It cancels the in-flight
    //  "send()" calls (their callers return promptly with the bytes written
    //  so far). The socket is left in State::DESTRUCTING; the destructor's
    //  close_and_wait() destroys the QTcpSocket and returns it to NOT_RUNNING.
    //  No "on_disconnect" callback is fired. Safe to call multiple times.
    void close() noexcept override;

    void connect(const std::string& address, uint16_t port) override;

    virtual size_t send(
        const void* data,
        size_t bytes,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    ) override;


signals:
    void internal_connect(const std::string& address, uint16_t port);
    void internal_send(QSharedPointer<SendData> send_data);


private:
    //  Stops the connection, destroys the QTcpSocket on its event thread, and
    //  returns the socket to State::NOT_RUNNING. It blocks until the
    //  QTcpSocket has been destroyed, so it must not be called from within a
    //  listener callback. The public "close()" only requests the teardown;
    //  this actually destroys the socket, and is called by the destructor.
    void close_and_wait() noexcept;

    //  The connection attempt or the connection itself ended in a failure.
    //  This fires the appropriate callback and returns the socket to
    //  State::NOT_RUNNING so "connect()" can be called again. If the
    //  connection was only tentatively confirmed (see the "connected" slot
    //  below), it is still reported as a failed connect attempt. Only the
    //  first of the (errorOccurred, disconnected) signals for a given
    //  failure fires a callback.
    //  Runs on the QTcpSocket's event thread.
    void connection_failed(const std::string& error_message);

    //  Cancels the in-flight "send()" calls: marks them done so their
    //  callers return promptly with the bytes written so far (best
    //  effort). The queued write loops finish without writing. The
    //  shared ownership keeps each packet alive until its queued call
    //  runs (or is discarded with the socket).
    void cancel_inflight_sends();

    std::unique_ptr<QTcpSocket> make_socket();


private:
    Mutex m_lock;

    QTcpSocket* m_socket = nullptr;
    bool m_success_pending = false;

    //  The "send()" calls that are queued on the event thread but not
    //  finished yet, so that "close()"/"connect()" can cancel them (and
    //  wake their callers) before the socket is destroyed. The write
    //  loops remove their entry when they finish. Protected by m_lock.
    std::vector<QSharedPointer<SendData>> m_inflight_sends;
};





}
#endif
