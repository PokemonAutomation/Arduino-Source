/*  Polling Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamConnections_PollingStreamConnections_H
#define PokemonAutomation_StreamConnections_PollingStreamConnections_H

#include <stddef.h>
#include "StreamInterface.h"

#ifdef PABB2_ENABLE
#include "PabbTime.h"
#else
#include "Common/Cpp/Time.h"
#endif

namespace PokemonAutomation{


class UnreliableStreamConnectionPolling : public UnreliableStreamSender{
public:
    virtual size_t unreliable_recv(void* data, size_t max_bytes, const WallDuration& timeout) noexcept = 0;
};


class ReliableStreamConnectionPolling{
public:
    //  Enqueue the specified data into the uncommitted stream.
    //  On success, returns true.
    //  On fail, return false and aborts all uncommitted sends.
    virtual bool enqueue_uncommitted_reliable_sends(const void* data, size_t bytes) noexcept = 0;

    //  Cancels all uncommitted sends.
    virtual void abort_uncommitted_reliable_sends() noexcept = 0;

    //  Commits all uncommitted sends to stream.
    virtual void commit_uncommitted_reliable_sends() noexcept = 0;

    virtual bool reliable_send_all_or_nothing(const void* data, size_t bytes) noexcept{
        if (!enqueue_uncommitted_reliable_sends(data, bytes)){
            return false;
        }
        commit_uncommitted_reliable_sends();
        return true;
    }


public:
    virtual size_t reliable_recv(void* data, size_t max_bytes) = 0;

    virtual bool reset_flag_set() const{ return false; }
    virtual void clear_reset_flag(){}

    virtual bool run_send_events(const WallDuration& timeout){
        return false;
    }
    virtual bool run_recv_events(const WallDuration& timeout){
        return false;
    }
};




}
#endif
