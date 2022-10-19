/*  CPU ID (arm64)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>

#include "CpuId.h"

namespace PokemonAutomation{

const char* PA_ARCH_STRING = "arm64";

const CPU_Features CPU_CAPABILITY_NATIVE;

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {"none", "Nothing (C++ Only)",             CPU_CAPABILITY_NATIVE,     true},
    };
    return LIST;
}




}


