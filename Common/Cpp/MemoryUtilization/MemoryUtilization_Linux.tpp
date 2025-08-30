/*  Memory Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MemoryUtilization_Linux_TPP
#define PokemonAutomation_MemoryUtilization_Linux_TPP

#include <stdint.h>
#include <utility>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "MemoryUtilization.h"

namespace PokemonAutomation{



uint64_t parse_integer(const char*& ptr){
    uint64_t ret = 0;

    //  Skip the white space.
    while (true){
        char ch = *ptr;
        switch (ch){
        case '\0':
            return ret;
        case ' ':
        case '\t':
            ptr++;
            continue;
        default:;
        }
        break;
    }

    //  Parse the integer.
    while (true){
        char ch = *ptr++;
        if (ch < '0' || ch > '9'){
            break;
        }
        ret *= 10;
        ret += ch - '0';
    }

    return ret;
}

MemoryUsage process_memory_usage(){
    MemoryUsage usage;

    uint64_t page_size = sysconf(_SC_PAGE_SIZE);

    usage.total_system_memory = (uint64_t)sysconf(_SC_PHYS_PAGES) * page_size;

    {
        uint64_t bytes = 0;

#if 1
        const std::string TOKEN = "MemAvailable:";

        std::ifstream file("/proc/meminfo");
        std::string line;
        while (std::getline(file, line)){
//            cout << line << endl;

            size_t pos = line.find(TOKEN);
            if (pos == std::string::npos){
                continue;
            }

            //  Skip the token.
            pos += TOKEN.size();

            const char* ptr = &line[pos];
            bytes = parse_integer(ptr) * 1024;

            break;
        }
#endif

        if (bytes == 0){
            bytes = (uint64_t)sysconf(_SC_AVPHYS_PAGES) * page_size;
        }
        bytes = std::min(bytes, usage.total_system_memory);

        usage.total_used_system_memory = usage.total_system_memory - bytes;
    }

    uint64_t swapped = 0;
    {
        const std::string VmRSS = "VmRSS:";
        const std::string VmSwap = "VmSwap:";

        pid_t current_pid = getpid();
        std::ifstream file("/proc/" + std::to_string(current_pid) + "/status");
        std::string line;
        while (std::getline(file, line)){
//            cout << line << endl;

            size_t pos = line.find(VmRSS);
            if (pos != std::string::npos){
                pos += VmRSS.size();
                const char* ptr = &line[pos];
                usage.process_physical_memory = parse_integer(ptr) * 1024;
                continue;
            }
            pos = line.find(VmSwap);
            if (pos != std::string::npos){
                pos += VmSwap.size();
                const char* ptr = &line[pos];
                swapped = parse_integer(ptr) * 1024;
                continue;
            }
        }
        usage.process_virtual_memory = usage.process_physical_memory + swapped;
    }

    return usage;
}




}
#endif
