/*  Server Socket 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The default AbstractServerSocket2 backend for the current platform,
 *  mirroring "ServerSocket.h". The Qt backend (ServerSocket2_Qt) exists
 *  for callers that need to integrate with a Qt event loop; it is not
 *  the default.
 *
 */

#ifndef PokemonAutomation_ServerSocket2_H
#define PokemonAutomation_ServerSocket2_H



#ifdef _WIN32
#include "ServerSocket2_WinSocket.h"
namespace PokemonAutomation{
    using ServerSocket2 = ServerSocket2_WinSocket;
}
#else
#include "ServerSocket2_POSIX.h"
namespace PokemonAutomation{
    using ServerSocket2 = ServerSocket2_POSIX;
}
#endif



#endif
