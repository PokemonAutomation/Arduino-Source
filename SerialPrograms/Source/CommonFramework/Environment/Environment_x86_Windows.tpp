/*  Environment (x86 Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <Windows.h>

#if __GNUC__
#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif
#endif

#include <map>
#include <thread>
#include <nmmintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Environment.h"

namespace PokemonAutomation{


uint64_t x86_rdtsc(){
    return __rdtsc();
}


uint64_t x86_measure_rdtsc_ticks_per_sec(){
    HANDLE thread = GetCurrentThread();

    GROUP_AFFINITY before_affinity;
    if (GetThreadGroupAffinity(thread, &before_affinity) == 0){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "GetThreadGroupAffinity() failed: Unable to read thread affinity.");
    }

    KAFFINITY t = 1;
    while ((t & before_affinity.Mask) == 0){
        t <<= 1;
    }

    GROUP_AFFINITY placeholder;
    GROUP_AFFINITY new_affinity = before_affinity;
    new_affinity.Mask = t;
    if (SetThreadGroupAffinity(thread, &new_affinity, &placeholder) == 0){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "SetThreadGroupAffinity() failed: Unable to set Affinity Mask.");

    }

    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency)){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "QueryPerformanceFrequency() failed: Unable to measure clock speed.");
    }
    uint64_t freq = frequency.QuadPart;
    freq >>= 4;


    uint64_t start_cycles = __rdtsc();

    LARGE_INTEGER start_timer;
    if (!QueryPerformanceCounter(&start_timer)){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "QueryPerformanceCounter() failed: Unable to measure clock speed.");
    }
    LARGE_INTEGER current_timer;
    do {
        if (!QueryPerformanceCounter(&current_timer)){
            throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "QueryPerformanceCounter() failed: Unable to measure clock speed.");
        }
    }while ((uint64_t)current_timer.QuadPart - (uint64_t)start_timer.QuadPart < freq);

    uint64_t end_cycles = __rdtsc();

    if (SetThreadGroupAffinity(thread, &before_affinity, &placeholder) == 0){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "SetThreadGroupAffinity() failed: Unable to set Affinity Mask.");
    }

    double cycle_dif = (double)(end_cycles - start_cycles);
    double timer_dif = (double)((uint64_t)current_timer.QuadPart - (uint64_t)start_timer.QuadPart);

    return (uint64_t)(cycle_dif / timer_dif * frequency.QuadPart);
}





ProcessorSpecs get_processor_specs(){
    ProcessorSpecs specs;
    specs.name = get_processor_name();
    specs.base_frequency = x86_rdtsc_ticks_per_sec();

    DWORD bytes = 0;
    GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP::RelationAll, nullptr, &bytes);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "GetLogicalProcessorInformationEx() failed.");
    }

    std::vector<char> ptr(bytes);
    if (!GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP::RelationAll, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)ptr.data(), &bytes)){
        throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "GetLogicalProcessorInformationEx() failed.");
    }

    std::map<size_t, KAFFINITY> group_masks;

    for (size_t c = 0; c < bytes;){
        const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX& info = *(const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(ptr.data() + c);
        switch (info.Relationship){
        case LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore:
            for (size_t g = 0; g < info.Processor.GroupCount; g++){
                const GROUP_AFFINITY& affinity = info.Processor.GroupMask[g];
                group_masks[affinity.Group] |= affinity.Mask;
            }
            specs.cores++;
            break;
        case LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorPackage:
            specs.sockets++;
            break;
        case LOGICAL_PROCESSOR_RELATIONSHIP::RelationNumaNode:
            specs.numa_nodes++;
            break;
        default:;
        }
        c += info.Size;
    }

    if (group_masks.size() == 0){
        specs.threads = std::thread::hardware_concurrency();
    }else{
        for (const auto& group : group_masks){
            specs.threads += _mm_popcnt_u64(group.second);
        }
    }

    return specs;
}





}
