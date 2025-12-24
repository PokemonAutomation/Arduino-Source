/*  CPU ID (arm64)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <stdint.h>
#include <sys/sysctl.h>

#include "CpuId.h"

namespace PokemonAutomation{

const char* PA_ARCH_STRING = "arm64";


uint64_t detect_NEON()
{
    // https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_instruction_set_characteristics
    uint64_t available = 0;
    size_t size = sizeof(available);

    if (sysctlbyname("hw.optional.AdvSIMD", &available, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
    return available;
}

CPU_Features& CPU_Features::set_to_current(){
    OK_M1 = detect_NEON() > 0;
    return *this;
}

CPU_Features make_M1(){
    CPU_Features ret;

    ret.OK_M1 = true;
    return ret;
}

// The CPU feature we use in the program will be C++ only, no SIMD
const CPU_Features& CPU_CAPABILITY_NOTHING(){
    static const CPU_Features ret;
    return ret;
}

// The CPU feature we use in the program will be those from Apple M1 architecture, including Arm NEON SIMD instruction set
const CPU_Features& CPU_CAPABILITY_M1(){
    static const CPU_Features ret = make_M1();
    return ret;
}

// We assume all kinds of Apple M1 chips has the same SIMD support.
// So they should all be the same CPU feature set as M1.
const CPU_Features& CPU_CAPABILITY_NATIVE(){
    static const CPU_Features ret = CPU_Features().set_to_current();
    return ret;
}

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {"none", "Nothing (C++ Only)",             CPU_CAPABILITY_NOTHING(),    true},
        {"m1-neon", "Apple M1 (NEON)",             CPU_CAPABILITY_M1(),         CPU_CAPABILITY_NATIVE().OK_M1},
    };
    return LIST;
}




}


