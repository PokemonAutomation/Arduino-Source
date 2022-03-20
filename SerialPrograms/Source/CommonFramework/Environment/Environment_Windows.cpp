/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <iostream>
#include <thread>
#include <nmmintrin.h>

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

#include <iostream>
using std::cout;
using std::endl;

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



extern const int THREAD_PRIORITY_MIN = -3;
extern const int THREAD_PRIORITY_MAX = 3;
const char* thread_priority_name(int priority){
    switch (priority){
    case -3:
        return "Idle Priority";
    case -2:
        return "Lowest Priority";
    case -1:
        return "Below Normal Priority";
    case 0:
        return "Normal Priority";
    case 1:
        return "Above Normal Priority";
    case 2:
        return "Highest Priority";
    case 3:
        return "Time Critical Priority";
    default:
        return nullptr;
    }
}
bool set_thread_priority(int priority){
    int native_priority;
    switch (priority){
    case -3:
        native_priority = THREAD_PRIORITY_IDLE;
        break;
    case -2:
        native_priority = THREAD_PRIORITY_LOWEST;
        break;
    case -1:
        native_priority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case 0:
        native_priority = THREAD_PRIORITY_NORMAL;
        break;
    case 1:
        native_priority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case 2:
        native_priority = THREAD_PRIORITY_HIGHEST;
        break;
    case 3:
        native_priority = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    default:
        if (priority < 0){
            native_priority = THREAD_PRIORITY_IDLE;
        }else{
            native_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    }
    if (!SetThreadPriority(GetCurrentThread(), native_priority)){
        DWORD error = GetLastError();
        std::cerr << "Unable to set thread priority. Error Code = " << error << std::endl;
        return false;
    }
    return true;
}
QThread::Priority to_qt_priority(int priority){
    switch (priority){
    case -3:
        return QThread::IdlePriority;
    case -2:
        return QThread::LowestPriority;
    case -1:
        return QThread::LowPriority;
    case 0:
        return QThread::NormalPriority;
    case 1:
        return QThread::HighPriority;
    case 2:
        return QThread::HighestPriority;
    case 3:
        return QThread::TimeCriticalPriority;
    default:
        if (priority < 0){
            return QThread::IdlePriority;
        }else{
            return QThread::TimeCriticalPriority;
        }
    }
}










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
#endif
