/*  Memory Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MemoryUtilization_Linux_TPP
#define PokemonAutomation_MemoryUtilization_Linux_TPP

#include <utility>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/proc_info.h>
#include "MemoryUtilization.h"

namespace PokemonAutomation{



MemoryUsage process_memory_usage(){
    MemoryUsage usage;


    usage.total_system_memory = (uint64_t)sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);

    {
        uint64_t bytes = 0;

        const std::string TOKEN = "MemAvailable:";

        std::ifstream file("/proc/meminfo");
        std::string line;
        while (std::getline(file, line)){
            size_t pos = line.find(TOKEN);
            if (pos == std::string::npos){
                continue;
            }

            //  Skip the token.
            pos += TOKEN.size();

            //  Skip the white space.
            while (line[pos] != '\0' && line[pos] == ' '){
                pos++;
            }

            //  Parse the integer.
            while (true){
                char ch = line[pos++];
                if (ch < '0' || ch > '9'){
                    break;
                }
                bytes *= 10;
                bytes += ch - '0';
            }
            bytes *= 1024;

            break;
        }

        if (bytes == 0){
            bytes = (uint64_t)sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
        }
        bytes = std::min(bytes, usage.total_system_memory);

        usage.total_used_system_memory = usage.total_system_memory - bytes;
    }

    pid_t pid = getpid();
    struct proc_taskinfo task_info;
    int ret = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));

    if (ret <= 0) {
        std::cerr << "Error getting process info for PID " << pid << ": " << strerror(errno) << std::endl;
    }else{
        usage.process_physical_memory = task_info.pti_resident_size;
    }

    return usage;
}




}
#endif
