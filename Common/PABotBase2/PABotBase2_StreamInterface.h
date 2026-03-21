/*  PABotBase2 Stream Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_StreamInterface_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_StreamInterface_H

#include <stddef.h>
#include <stdint.h>

namespace PokemonAutomation{
namespace PABotBase2{


class StreamConnection{
public:

    virtual size_t send(const void* data, size_t bytes, bool is_retransmit = false) = 0;
    virtual size_t recv(void* data, size_t max_bytes) = 0;
    
    virtual bool run_events(){
        return false;
    }
    virtual bool reset_flag_set() const{ return false; }
    virtual void clear_reset_flag(){}

    //  Wait for something to be ready to receive. Timeout in milliseconds.
    virtual void wait_for_recv_available(uint16_t milliseconds){
        (void)milliseconds;
    }
};




}
}
#endif
