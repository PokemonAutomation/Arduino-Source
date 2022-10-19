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

namespace PokemonAutomation {

const char* PA_ARCH_STRING = "Unknown";

const CPU_Features CPU_CAPABILITY_NATIVE;
CPU_Features CPU_CAPABILITY_CURRENT;

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {"Nothing (C++ Only)",             CPU_CAPABILITY_NOTHING,     true},
    };
    return LIST;
}

}

#endif

namespace PokemonAutomation {

IntegerEnumDatabase make_CAPABILITIES_DATABASE(){
    IntegerEnumDatabase ret;
    size_t c = 0;
    for (const CpuCapabilityOption& item : AVAILABLE_CAPABILITIES()){
        ret.add(c, item.slug, item.display, item.available);
        c++;
    }
    return ret;
};
const IntegerEnumDatabase& CAPABILITIES_DATABASE(){
    static const IntegerEnumDatabase database = make_CAPABILITIES_DATABASE();
    return database;
}

}

