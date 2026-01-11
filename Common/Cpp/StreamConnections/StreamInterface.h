/*  Stream Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamInterface_H
#define PokemonAutomation_StreamInterface_H

namespace PokemonAutomation{


struct StreamSender{
    virtual size_t send(const void* data, size_t bytes) = 0;
};
struct StreamListener{
    virtual void on_recv(const void* data, size_t bytes) = 0;
};


}
#endif
