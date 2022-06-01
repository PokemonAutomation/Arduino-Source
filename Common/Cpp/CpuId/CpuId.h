/*  CPU ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuId_H
#define PokemonAutomation_CpuId_H

#if 0
#elif _M_IX86 || _M_X64 || __i386__ || __x86_64__
#include "CpuId_x86.h"
#else
namespace PokemonAutomation{
    struct CPU_Features{};
}
#endif


#include <vector>

namespace PokemonAutomation{


extern const CPU_Features CPU_CAPABILITY_NATIVE;
extern CPU_Features CPU_CAPABILITY_CURRENT;



struct CpuCapabilityOption{
    const char* label;
    const CPU_Features& features;
    bool available;

    CpuCapabilityOption(const char* p_label, const CPU_Features& p_features, bool p_available)
        : label(p_label)
        , features(p_features)
        , available(p_available)
    {}
};
const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES();




}
#endif
