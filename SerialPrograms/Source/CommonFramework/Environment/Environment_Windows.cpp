/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef _WIN32
#include <Windows.h>
#include "Common/Cpp/Exception.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "Environment.h"

#if __GNUC__
#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif
#endif

namespace PokemonAutomation{

#if __GNUC__
void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx){
    __cpuid_count(eax, ecx, eabcdx[0], eabcdx[1], eabcdx[2], eabcdx[3]);
}
#else
void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx){
    int out[4];
    __cpuidex(out, eax, ecx);
    eabcdx[0] = out[0];
    eabcdx[1] = out[1];
    eabcdx[2] = out[2];
    eabcdx[3] = out[3];
}
#endif

uint64_t x86_rdtsc(){
    return __rdtsc();
}
uint64_t x86_measure_rdtsc_ticks_per_sec(){
    HANDLE thread = GetCurrentThread();

    GROUP_AFFINITY before_affinity;
    if (GetThreadGroupAffinity(thread, &before_affinity) == 0){
        global_logger_tagged().log("Unable to read thread affinity.");
        PA_THROW_StringException("Unable to read thread affinity.");
    }

    KAFFINITY t = 1;
    while ((t & before_affinity.Mask) == 0){
        t <<= 1;
    }

    GROUP_AFFINITY placeholder;
    GROUP_AFFINITY new_affinity = before_affinity;
    new_affinity.Mask = t;
    if (SetThreadGroupAffinity(thread, &new_affinity, &placeholder) == 0){
        global_logger_tagged().log("Unable to set Affinity Mask.");
        PA_THROW_StringException("Unable to set Affinity Mask.");
    }

    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency)){
        global_logger_tagged().log("Unable to measure clock speed.");
        PA_THROW_StringException("Unable to measure clock speed.");
    }
    uint64_t freq = frequency.QuadPart;
    freq >>= 4;


    uint64_t start_cycles = __rdtsc();

    LARGE_INTEGER start_timer;
    if (!QueryPerformanceCounter(&start_timer)){
        global_logger_tagged().log("Unable to measure clock speed.");
        PA_THROW_StringException("Unable to measure clock speed.");
    }
    LARGE_INTEGER current_timer;
    do {
        if (!QueryPerformanceCounter(&current_timer)){
            global_logger_tagged().log("Unable to measure clock speed.");
            PA_THROW_StringException("Unable to measure clock speed.");
        }
    }while ((uint64_t)current_timer.QuadPart - (uint64_t)start_timer.QuadPart < freq);

    uint64_t end_cycles = __rdtsc();

    if (SetThreadGroupAffinity(thread, &before_affinity, &placeholder) == 0){
        global_logger_tagged().log("Unable to set Affinity Mask.");
        PA_THROW_StringException("Unable to set Affinity Mask.");
    }

    double cycle_dif = (double)(end_cycles - start_cycles);
    double timer_dif = (double)((uint64_t)current_timer.QuadPart - (uint64_t)start_timer.QuadPart);

    return (uint64_t)(cycle_dif / timer_dif * frequency.QuadPart);
}





}
#endif
