/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Environment.h"

#if _WIN32
#include "Environment_Windows.tpp"
#endif

#if defined(__linux) || defined(__APPLE__)
#include "Environment_Linux.tpp"
#endif


namespace PokemonAutomation{






}
