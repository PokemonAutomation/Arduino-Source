/*  Server Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_H
#define PokemonAutomation_ServerSocket_H



#ifdef _WIN32
#include "ServerSocket_Qt.h"
namespace PokemonAutomation{
    using ServerSocket = ServerSocket_Qt;
}
#else
#include "ServerSocket_POSIX.h"
namespace PokemonAutomation{
    using ServerSocket = ServerSocket_POSIX;
}
#endif



#endif
