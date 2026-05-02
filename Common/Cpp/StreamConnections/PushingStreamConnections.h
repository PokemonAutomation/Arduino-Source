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


class StreamConnectionPushing{
public:
    virtual ~StreamConnectionPushing(){}
    virtual void stop(){};

    void add_listener(StreamListener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(StreamListener& listener){
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
    virtual bool reliable_send_all_or_nothing(
        const void* data, size_t bytes,
        WallDuration timeout
    ) noexcept = 0;

protected:
    void on_reliable_recv(const void* data, size_t bytes){
        m_listeners.run_method(&StreamListener::on_recv, data, bytes);
    }
};




}
#endif
