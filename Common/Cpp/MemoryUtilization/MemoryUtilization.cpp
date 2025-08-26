#include <iostream>

#if defined(__linux) || defined(__APPLE__)
// get process phsyical memory for Linux and macOS
#include <libproc.h>
#include <sys/proc_info.h>
#include <unistd.h> // For getpid()
#endif

#if defined(__APPLE__)
// get system memory and process virutal memory for macOS
#include <sys/sysctl.h>
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/types.h>
#endif

#include "MemoryUtilization.h"

namespace PokemonAutomation{



/**
 * @brief Retrieves and analyzes the virtual memory regions of the current process.
 */
void analyze_vm_regions() {
    kern_return_t kr;
    mach_port_t task = mach_task_self();

    mach_vm_address_t address = 0;
    mach_vm_size_t size = 0;
    
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
    
    mach_port_t object_name;
    
    // Total virtual memory usage
    mach_vm_size_t total_size = 0;
    
    std::cout << "Analyzing virtual memory regions for current process..." << std::endl;
    
    // Loop through all virtual memory regions
    while (true) {
        kr = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &info_count, &object_name);
        
        if (kr == KERN_INVALID_ADDRESS) {
            // Reached the end of the address space
            break;
        } else if (kr != KERN_SUCCESS) {
            std::cerr << "mach_vm_region failed with error: " << kr << std::endl;
            break;
        }

        // Add region size to total
        total_size += size;

        // Print details of the current region
        std::cout << "Region at address " << (void*)address 
                  << ", size " << size << " bytes"
                  << ", protection: ";
        
        if (info.protection & VM_PROT_READ) {
            std::cout << "R";
        }
        if (info.protection & VM_PROT_WRITE) {
            std::cout << "W";
        }
        if (info.protection & VM_PROT_EXECUTE) {
            std::cout << "X";
        }
        std::cout << std::endl;
        
        // Move to the next region
        address += size;
    }
    
    std::cout << "\nTotal virtual memory allocated: " << total_size << " bytes" << std::endl;
    std::cout << "Approximately " << (double)total_size / 1048576.0 << " MB" << std::endl;
}


MemoryUsage process_memory_usage(){
    MemoryUsage usage;

#if defined(__linux) || defined(__APPLE__)
    pid_t pid = getpid();
    struct proc_taskinfo task_info;
    int ret = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));

    if (ret <= 0) {
        std::cerr << "Error getting process info for PID " << pid << ": " << strerror(errno) << std::endl;
    }else{
        usage.process_physical_memory = task_info.pti_resident_size;
    }
    
#if defined(__APPLE__) // compute rest of the memory info on macOS

    int mib[] = {CTL_HW, HW_MEMSIZE};
    int64_t physical_memory = 0;
    size_t length = sizeof(physical_memory);

    if (sysctl(mib, 2, &physical_memory, &length, NULL, 0) != 0) {
        std::cerr << "Error calling sysctl()." << std::endl;
    } else{
        usage.total_system_memory = physical_memory;
    }

    vm_size_t page_size;
    vm_statistics_data_t vm_stats;
    mach_port_t mach_port = mach_host_self();
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(integer_t);
    
    // Get the host statistics
    if (KERN_SUCCESS != host_statistics(mach_port, HOST_VM_INFO, (host_info_t)&vm_stats, &count)) {
        std::cerr << "Failed to get host statistics." << std::endl;
    } else{
        // Get the system's page size
        host_page_size(mach_port, &page_size);

        // Calculate used memory from vm_statistics
        // Used memory = Wired + Active + Inactive
        size_t wired_pages = vm_stats.wire_count;
        size_t active_pages = vm_stats.active_count;
        size_t inactive_pages = vm_stats.inactive_count;

        usage.total_used_system_memory = (wired_pages + active_pages + inactive_pages) * page_size;
    }
    
#else  // compute total used system memory on Linux
    // TODO
#endif

#else // compute memory info on Windows
    // TODO
#endif
    return usage;
}


}