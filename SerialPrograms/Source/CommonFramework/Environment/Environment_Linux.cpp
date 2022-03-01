/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#if defined(__linux) || defined(__APPLE__)

#include <chrono>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "Environment.h"

#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif

namespace PokemonAutomation{


void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx){
    __cpuid_count(eax, ecx, eabcdx[0], eabcdx[1], eabcdx[2], eabcdx[3]);
}
uint64_t x86_rdtsc(){
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
uint64_t x86_measure_rdtsc_ticks_per_sec(){
//    Time::WallClock w_start = Time::WallClock::Now();
    auto w_start = std::chrono::system_clock::now();
    uint64_t r_start = x86_rdtsc();
    while (std::chrono::system_clock::now() - w_start < std::chrono::microseconds(62500));
    auto w_end = std::chrono::system_clock::now();
//    while (w_start.SecondsElapsed() < 0.0625);
//    Time::WallClock w_end = Time::WallClock::Now();
    uint64_t r_end = x86_rdtsc();

    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(w_end - w_start);
    double seconds = (double)elapsed.count() / 1000000.;

    return (uint64_t)((double)(r_end - r_start) / seconds);
}



}
#endif
