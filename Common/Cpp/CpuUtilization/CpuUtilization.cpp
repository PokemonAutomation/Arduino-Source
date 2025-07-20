/*  CPU Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CpuUtilization.h"


#if _WIN32
#include "CpuUtilization_Windows.tpp"
#endif

#if defined(__linux) || defined(__APPLE__)
#include "CpuUtilization_Linux.tpp"
#endif
