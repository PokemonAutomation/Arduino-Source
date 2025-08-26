/*  Memory Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MemoryUtilization_H
#define PokemonAutomation_MemoryUtilization_H

#include <cstddef>

namespace PokemonAutomation{


struct MemoryUsage{
    size_t total_system_memory = 0; // in bytes
    size_t total_used_system_memory = 0;  // in bytes
    size_t process_physical_memory = 0;  // in bytes
    size_t process_virtual_memory = 0;  // in bytes
};


MemoryUsage process_memory_usage();


}


#endif