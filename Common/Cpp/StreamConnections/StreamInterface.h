/*  Stream Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamConnections_StreamInterface_H
#define PokemonAutomation_StreamConnections_StreamInterface_H

namespace PokemonAutomation{


struct UnreliableStreamSender{
    virtual size_t unreliable_send(const void* data, size_t bytes, bool is_retransmit) = 0;
};


struct StreamSender{
    virtual size_t send(const void* data, size_t bytes) = 0;
};
struct StreamListener{
    virtual void on_recv(const void* data, size_t bytes) = 0;
};


}
#endif
