/*  CPU ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CpuId.h"

#if 0
#elif defined PokemonAutomation_CpuId_x86_H
#include "CpuId_x86.tpp"
#elif defined PokemonAutomation_CpuId_arm64_H
#include "CpuId_arm64.tpp"
#else

namespace PokemonAutomation{

const char* PA_ARCH_STRING = "Unknown";

const CPU_Features CPU_CAPABILITY_NATIVE;

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {"none", "Nothing (C++ Only)",             CPU_CAPABILITY_NATIVE,     true},
    };
    return LIST;
}

}

#endif

namespace PokemonAutomation{

CPU_Features CPU_CAPABILITY_CURRENT = CPU_CAPABILITY_NATIVE;

IntegerEnumDropdownDatabase make_CAPABILITIES_DATABASE(){
    IntegerEnumDropdownDatabase ret;
    size_t c = 0;
    for (const CpuCapabilityOption& item : AVAILABLE_CAPABILITIES()){
        ret.add(c, item.slug, item.display, item.available);
        c++;
    }
    return ret;
};
const IntegerEnumDropdownDatabase& CAPABILITIES_DATABASE(){
    static const IntegerEnumDropdownDatabase database = make_CAPABILITIES_DATABASE();
    return database;
}

}

