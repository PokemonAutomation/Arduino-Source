/*  Stream Connection Interface
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_StreamConnection_H
#define PokemonAutomation_StreamConnection_H

#include "Common/Cpp/ListenerSet.h"
#include "StreamInterface.h"

namespace PokemonAutomation{


class StreamConnection : public StreamSender{
public:
    virtual ~StreamConnection(){}
    virtual void stop(){};

    void add_listener(StreamListener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(StreamListener& listener){
        m_listeners.remove(listener);
    }

protected:
    void on_recv(const void* data, size_t bytes){
        m_listeners.run_method(&StreamListener::on_recv, data, bytes);
    }

protected:
    ListenerSet<StreamListener> m_listeners;
};




}

#endif
