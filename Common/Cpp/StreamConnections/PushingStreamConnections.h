/*  Pushing Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamConnections_PushingStreamConnections_H
#define PokemonAutomation_StreamConnections_PushingStreamConnections_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/ListenerSet.h"
#include "StreamInterface.h"

namespace PokemonAutomation{

class Cancellable;


class StreamConnectionPushing{
public:
    virtual ~StreamConnectionPushing(){}
    virtual void stop() noexcept{};

    void add_listener(StreamListener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(StreamListener& listener) noexcept{
        m_listeners.remove(listener);
    }

protected:
    ListenerSet<StreamListener> m_listeners;
};


class UnreliableStreamConnectionPushing : public StreamConnectionPushing, public UnreliableStreamSender{
protected:
    void on_unreliable_recv(const void* data, size_t bytes){
        m_listeners.run_method(&StreamListener::on_recv, data, bytes);
    }
};


class ReliableStreamConnectionPushing : public StreamConnectionPushing{
public:
    //  Throws exception if cancelled or connection is dead.
    virtual void reliable_send_all_or_nothing(
        Cancellable* cancellable,
        const void* data, size_t bytes
    ) = 0;

    //  Returns false if cannot send within timeout.
    //  Throws exception if cancelled or connection is dead.
    [[nodiscard]] virtual bool reliable_send_all_or_nothing(
        Cancellable* cancellable,
        const void* data, size_t bytes,
        WallClock deadline
    ) = 0;

protected:
    void on_reliable_recv(const void* data, size_t bytes){
        m_listeners.run_method(&StreamListener::on_recv, data, bytes);
    }
};




}
#endif
