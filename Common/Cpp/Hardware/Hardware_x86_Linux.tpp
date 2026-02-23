/*  Environment (x86 Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <map>
#include <thread>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/Logging/Logger.h"
#include "Hardware.h"

#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif

namespace PokemonAutomation{


uint64_t x86_rdtsc(){
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}



uint64_t x86_measure_rdtsc_ticks_per_sec(){
//    Time::WallClock w_start = Time::WallClock::Now();
    auto w_start = current_time();
    uint64_t r_start = x86_rdtsc();
    while (current_time() - w_start < std::chrono::microseconds(62500));
    auto w_end = current_time();
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
