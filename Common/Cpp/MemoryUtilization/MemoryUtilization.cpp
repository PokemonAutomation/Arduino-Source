/*  Memory Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "MemoryUtilization.h"

#if 0

#elif _WIN32
#include "MemoryUtilization_Windows.tpp"

#elif __APPLE__
#include "MemoryUtilization_Mac.tpp"

#elif __linux
#include "MemoryUtilization_Linux.tpp"

#else

namespace PokemonAutomation{
    MemoryUsage process_memory_usage(){
        return MemoryUsage{};
    }
}

#endif


