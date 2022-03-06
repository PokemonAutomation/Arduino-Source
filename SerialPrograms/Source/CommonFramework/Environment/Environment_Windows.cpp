/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "Environment.h"

#if __GNUC__
#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif
#endif

namespace PokemonAutomation{





extern const int DEFAULT_PRIORITY_INDEX = 2;
const std::vector<QString> PRIORITY_MODES{
    "Realtime",
    "High",
    "Above Normal",
    "Normal",
    "Below Normal",
    "Low",
};
int priority_name_to_index(const QString& name){
    for (size_t c = 0; c < PRIORITY_MODES.size(); c++){
        if (name == PRIORITY_MODES[c]){
            return (int)c;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority String: " + name.toStdString());
}
bool set_priority_by_index(int index){
    DWORD native_priority = NORMAL_PRIORITY_CLASS;
    switch (index){
    case 0:
        native_priority = REALTIME_PRIORITY_CLASS;
        break;
    case 1:
        native_priority = HIGH_PRIORITY_CLASS;
        break;
    case 2:
        native_priority = ABOVE_NORMAL_PRIORITY_CLASS;
        break;
    case 3:
        native_priority = NORMAL_PRIORITY_CLASS;
        break;
    case 4:
        native_priority = BELOW_NORMAL_PRIORITY_CLASS;
        break;
    case 5:
        native_priority = IDLE_PRIORITY_CLASS;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority Index: " + std::to_string(index));
    }
    if (SetPriorityClass(GetCurrentProcess(), native_priority)){
        global_logger_tagged().log("Process priority set to: " + PRIORITY_MODES[index], COLOR_BLUE);
        return true;
    }
    DWORD error = GetLastError();
    global_logger_tagged().log("Unable to set process priority. Error Code = " + std::to_string(error), COLOR_RED);
    return false;
}
bool set_priority_by_name(const QString& name){
    int index = priority_name_to_index(name);
    return set_priority_by_index(index);
}
int read_priority_index(){
    switch (GetPriorityClass(GetCurrentProcess())){
    case REALTIME_PRIORITY_CLASS:
        return 0;
    case HIGH_PRIORITY_CLASS:
        return 1;
    case ABOVE_NORMAL_PRIORITY_CLASS:
        return 2;
    case NORMAL_PRIORITY_CLASS:
        return 3;
    case BELOW_NORMAL_PRIORITY_CLASS:
        return 4;
    case IDLE_PRIORITY_CLASS:
        return 5;
    default:
        DWORD error = GetLastError();
        std::cerr << "Unable to read process priority. Error Code = " << error << std::endl;
    }
    return -1;
}






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





}
#endif
