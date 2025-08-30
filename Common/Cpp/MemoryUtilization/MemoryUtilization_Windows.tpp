/*  Memory Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MemoryUtilization_Mac_TPP
#define PokemonAutomation_MemoryUtilization_Mac_TPP

#include <Windows.h>
#include <psapi.h>
#include "MemoryUtilization.h"

namespace PokemonAutomation{



MemoryUsage process_memory_usage(){
    MemoryUsage ret;
    {
        MEMORYSTATUSEX data;
        data.dwLength = sizeof(data);
        if (GlobalMemoryStatusEx(&data)){
            ret.total_system_memory = data.ullTotalPhys;
            ret.total_used_system_memory = data.ullTotalPhys - data.ullAvailPhys;
        }
    }
    {
        PROCESS_MEMORY_COUNTERS_EX2 data;
        if (GetProcessMemoryInfo(
            GetCurrentProcess(),
            (PROCESS_MEMORY_COUNTERS*)&data,
            sizeof(data)
        )){
            ret.process_physical_memory = data.PrivateWorkingSetSize;
            ret.process_virtual_memory = data.PagefileUsage;
        }
    }
    {
        ULONGLONG data;
        if (GetPhysicallyInstalledSystemMemory(&data)){
            ret.total_system_memory = data * 1024;
        }
    }
    return ret;
}


}
#endif
