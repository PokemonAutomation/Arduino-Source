/*  Client Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ClientSocket_H
#define PokemonAutomation_ClientSocket_H



#ifdef _WIN32
#include "ClientSocket_WinSocket.h"
namespace PokemonAutomation{
    using ClientSocket = ClientSocket_WinSocket;
}
#else
#include "ClientSocket_POSIX.h"
namespace PokemonAutomation{
    using ClientSocket = ClientSocket_POSIX;
}
#endif



#endif
