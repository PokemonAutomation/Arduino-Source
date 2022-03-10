/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#if defined(__linux) || defined(__APPLE__)

#include <chrono>
#include <set>
#include <map>
#include <thread>
#include <fstream>
#include <nmmintrin.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "Environment.h"

#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif

namespace PokemonAutomation{



extern const int DEFAULT_PRIORITY_INDEX = 1;
const std::vector<QString> PRIORITY_MODES{
    "Max Priority",
    "Min Priority",
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
    int native_priority = sched_get_priority_min(SCHED_RR);
    switch (index){
    case 0:
        native_priority = sched_get_priority_max(SCHED_RR);
        break;
    case 1:
        native_priority = sched_get_priority_min(SCHED_RR);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority Index: " + std::to_string(index));
    }

    struct sched_param param;
    param.sched_priority = native_priority;
    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0){
        return true;
    }

    int errorcode = errno;
    global_logger_tagged().log("Unable to set process priority. Error Code = " + std::to_string(errorcode), COLOR_RED);
    return false;
}
int read_priority_index(){
    return -1;
}



uint64_t x86_rdtsc(){
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}



void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx){
    __cpuid_count(eax, ecx, eabcdx[0], eabcdx[1], eabcdx[2], eabcdx[3]);
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





ProcessorSpecs get_processor_specs(){
    ProcessorSpecs specs;
    specs.name = get_processor_name();
    specs.base_frequency = x86_rdtsc_ticks_per_sec();
    specs.threads = std::thread::hardware_concurrency();

#ifdef __linux
    //  Cores + Sockets
    {
//        std::set<int> cores;
        std::set<int> sockets;

        int current_socket = 0;
        std::map<int, std::set<int>> cores_per_socket;

        std::ifstream file("/proc/cpuinfo");
        std::string line;
        while (std::getline(file, line)){
            if (line.find("physical id") == 0){
                size_t pos = line.find(": ");
                if (pos == std::string::npos){
                    throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Unable to parse: /proc/cpuinfo");
                }
                current_socket = atoi(&line[pos + 2]);
                sockets.insert(current_socket);
            }
            if (line.find("core id") == 0){
                size_t pos = line.find(": ");
                if (pos == std::string::npos){
                    throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Unable to parse: /proc/cpuinfo");
                }
//                specs.threads++;
                int core = atoi(&line[pos + 2]);
                cores_per_socket[current_socket].insert(core);
//                cores.insert(core);
            }
        }
        specs.sockets = sockets.size();
        specs.cores = 0;
        for (const auto& socket : cores_per_socket){
            specs.cores += socket.second.size();
        }
    }

    //  NUMA Nodes
    {
        std::set<int> nodes;

        std::ifstream file("/proc/zoneinfo");
        std::string line;
        while (std::getline(file, line)){
            if (line.find("Node ") == 0){
                nodes.insert(atoi(&line[5]));
            }
        }
        specs.numa_nodes = nodes.size();
    }
#endif

    return specs;
}




























}
#endif
