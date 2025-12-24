/*  CPU ID (arm64)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <stdint.h>
#include <sys/sysctl.h>
#include <iostream>

#include "CpuId.h"

namespace PokemonAutomation{

const char* PA_ARCH_STRING = "arm64";


uint64_t detect_NEON()
{
    // https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_instruction_set_characteristics
    // But this developer webpage may be out of date.
    
    uint64_t available = 0;
    size_t size = sizeof(available);

    // Since certain macOS version "hw.optional.AdvSIMD" no longer works, so to be safe
    // we check all three sysctl attributes.
    if (sysctlbyname("hw.optional.AdvSIMD", &available, &size, NULL, 0) < 0){
        if (sysctlbyname("hw.optional.neon", &available, &size, NULL, 0) < 0){
            if (sysctlbyname("hw.optional.arm.AdvSIMD", &available, &size, NULL, 0) < 0){
                std::cerr << "Error querying ARM SIMD set availability using macOS sysctl API" << std::endl;
                return 0;
            }
        }
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
const CPU_Features CPU_CAPABILITY_NOTHING;

// The CPU feature we use in the program will be those from Apple M1 architecture, including Arm NEON SIMD instruction set
const CPU_Features CPU_CAPABILITY_M1 = make_M1();

// We assume all kinds of Apple M1 chips has the same SIMD support.
// So they should all be the same CPU feature set as M1.
const CPU_Features CPU_CAPABILITY_NATIVE = CPU_Features().set_to_current();

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {"none", "Nothing (C++ Only)",             CPU_CAPABILITY_NOTHING,     true},
        {"m1-neon", "Apple M1 (NEON)",             CPU_CAPABILITY_M1,     CPU_CAPABILITY_NATIVE.OK_M1},
    };
    return LIST;
}




}


