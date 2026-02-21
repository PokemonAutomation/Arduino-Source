/*  Environment (x86)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CpuId/CpuId_x86.h"
#include "Hardware.h"

namespace PokemonAutomation{


uint64_t x86_measure_rdtsc_ticks_per_sec();


void x86_CleanCPUName(char name[49]){
    size_t c0 = 0;
    size_t c1 = 0;
    bool space_flag = true;
    while (c1 < 48){
        if (space_flag && name[c1] == ' '){
            c1++;
            continue;
        }
        space_flag = name[c1] == ' ';
        name[c0++] = name[c1++];
    }

    while (c0 < 48){
        name[c0++] = '\0';
    }
}
std::string get_processor_name(){
    union{
        uint32_t reg[12];
        char name[49];
    };
    x86_cpuid(reg + 0, 0x80000002, 0);
    x86_cpuid(reg + 4, 0x80000003, 0);
    x86_cpuid(reg + 8, 0x80000004, 0);

    x86_CleanCPUName(name);
    return name;
}
uint64_t x86_rdtsc_ticks_per_sec(){
    static uint64_t cached = x86_measure_rdtsc_ticks_per_sec();
    return cached;
}



}
