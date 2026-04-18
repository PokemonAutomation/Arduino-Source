/*  Polling Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamConnections_PollingStreamConnections_H
#define PokemonAutomation_StreamConnections_PollingStreamConnections_H

#include <stddef.h>
#include <stdint.h>
#include "StreamInterface.h"

#ifdef PABB2_ENABLE
#include "PabbTime.h"
#else
#include "Common/Cpp/Time.h"
#endif

namespace PokemonAutomation{


class UnreliableStreamConnectionPolling : public UnreliableStreamSender{
public:
    virtual size_t unreliable_recv(void* data, size_t max_bytes) = 0;

    //  Wait for something to be ready to receive.
    virtual void wait_for_recv_available(WallDuration timeout){
        (void)timeout;
    }
};


class ReliableStreamConnectionPolling{
public:
    virtual void reliable_send(const void* data, size_t bytes) = 0;
    virtual size_t reliable_recv(void* data, size_t max_bytes) = 0;

    virtual bool reset_flag_set() const{ return false; }
    virtual void clear_reset_flag(){}

    //  Wait for the next time to run an event.
    virtual void wait_for_event(WallDuration timeout){
        (void)timeout;
    }

    virtual bool run_events(){
        return false;
    }
};




}
#endif
