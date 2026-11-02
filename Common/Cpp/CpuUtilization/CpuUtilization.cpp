/*  CPU Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */



#if _WIN32
#include "CpuUtilization_Windows.tpp"
#endif

#if defined(__linux) || defined(__APPLE__)
#include "CpuUtilization_Linux.tpp"
#endif
