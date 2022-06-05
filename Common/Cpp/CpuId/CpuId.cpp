/*  CPU ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CpuId.h"

#if 0
#elif defined PokemonAutomation_CpuId_x86_H
#include "CpuId_x86.tpp"
#else
const char* PA_ARCH_STRING = "Unknown";

const CPU_Features CPU_CAPABILITY_NATIVE;
CPU_Features CPU_CAPABILITY_CURRENT;

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {"Nothing (C++ Only)",             CPU_CAPABILITY_NOTHING,     true},
    };
    return LIST;
}

#endif



