/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "Common/Cpp/CpuId.h"
#include "Environment.h"

namespace PokemonAutomation{



bool set_priority_by_name(const QString& name){
    int index = priority_name_to_index(name);
    return set_priority_by_index(index);
}

int clip_priority(int priority){
    priority = std::min(priority, THREAD_PRIORITY_MAX);
    priority = std::max(priority, THREAD_PRIORITY_MIN);
    return priority;
}




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
