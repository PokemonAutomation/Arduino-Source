/*  CPU ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuId_H
#define PokemonAutomation_CpuId_H

#include "Common/Cpp/EnumDatabase.h"

#if 0
#elif _M_IX86 || _M_X64 || __i386__ || __x86_64__
#define PA_ARCH_x86 1
#include "CpuId_x86.h"
#else
namespace PokemonAutomation{
    struct CPU_Features{};
}
#endif


#include <vector>

namespace PokemonAutomation{


extern const char* PA_ARCH_STRING;


extern const CPU_Features CPU_CAPABILITY_NATIVE;
extern CPU_Features CPU_CAPABILITY_CURRENT;



struct CpuCapabilityOption{
    const char* slug;
    const char* display;
    const CPU_Features& features;
    bool available;

    CpuCapabilityOption(
        const char* p_slug, const char* p_display,
        const CPU_Features& p_features, bool p_available
    )
        : slug(p_slug)
        , display(p_display)
        , features(p_features)
        , available(p_available)
    {}
};
const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES();
const IntegerEnumDatabase& CAPABILITIES_DATABASE();



}
#endif
