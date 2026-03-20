/*  PABotBase2 Stream Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_StreamInterface_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_StreamInterface_H

#include <stdint.h>

namespace PokemonAutomation{
namespace PABotBase2{


struct StreamConnection{
    virtual size_t send(const void* data, size_t bytes, bool is_retransmit) = 0;
    virtual size_t recv(void* data, size_t max_bytes) = 0;

    //  Wait for something to be ready to receive. Timeout in milliseconds.
    virtual void wait_for_recv_available(uint16_t milliseconds){
        (void)milliseconds;
    }
};




}
}
#endif
