/*  CPU ID
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuId_H
#define PokemonAutomation_CpuId_H

#include "Common/Cpp/Options/EnumDropdownDatabase.h"

#if 0
#elif _M_IX86 || _M_X64 || __i386__ || __x86_64__
#define PA_ARCH_x86 1
#include "CpuId_x86.h"
#elif __arm64__
#define PA_ARCH_arm64 1
#include "CpuId_arm64.h"
#else
namespace PokemonAutomation{
    struct CPU_Features{};
}
#endif


#include <vector>

namespace PokemonAutomation{

// CPU arch name, for Intel CPU it is "x64" or x86
extern const char* PA_ARCH_STRING;

// The running machine's CPU capability
extern const CPU_Features CPU_CAPABILITY_NATIVE;
// The CPU capability used to determine what Pokemon Automation code to run.
// e.g. to run with or without Intel AVX instructions.
// This capability is intialized to be the same as `CPU_CAPABILITY_NATIVE`.
// The reason we define two capability, native and current, is that at runtime
// we can chagne the values in current to test the code that runs with weaker capability.
extern CPU_Features CPU_CAPABILITY_CURRENT;

// Struct for a set of CPU features.
// This struct is used to build UI that lets user set different CPU features
// to test the code that runs with weaker capability.
// For example, on a machine with Intel AVX and AVX2 instructions, there can be three
// `CpuCapabilityOption`, representing non-AVX (pure C++), AVX and AVX2 implementations
// of some computation. The user can choose between those options to test the implementations,
// profile and compare their performance.
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

// The available CPU instruction choices. See comments of `CpuCapabilityOption` for more details.
const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES();
// An enum database to select `CpuCapabilityOption`.
// This database is built by `AVAILABLE_CAPABILITIES()`, used for UI of choosing CPU instructions.
const IntegerEnumDropdownDatabase& CAPABILITIES_DATABASE();



}
#endif
