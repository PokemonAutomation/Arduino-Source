/*  CPU Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuUtilization_H
#define PokemonAutomation_CpuUtilization_H


#if _WIN32
#include "CpuUtilization_Windows.h"
#elif defined(__linux) || defined(__APPLE__)
#include "CpuUtilization_Linux.h"
#else
#error "Unsupported platform."
#endif



#endif
