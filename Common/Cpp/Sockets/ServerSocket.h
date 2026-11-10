/*  Server Socket
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The default AbstractServerSocket backend for the current platform,
 *  mirroring "ClientSocket.h". The Qt backend (ServerSocket_Qt) exists
 *  for callers that need to integrate with a Qt event loop; it is not
 *  the default.
 *
 */

#ifndef PokemonAutomation_ServerSocket_H
#define PokemonAutomation_ServerSocket_H



#ifdef _WIN32
#include "ServerSocket_WinSocket.h"
namespace PokemonAutomation{
    using ServerSocket = ServerSocket_WinSocket;
}
#else
#include "ServerSocket_POSIX.h"
namespace PokemonAutomation{
    using ServerSocket = ServerSocket_POSIX;
}
#endif



#endif
