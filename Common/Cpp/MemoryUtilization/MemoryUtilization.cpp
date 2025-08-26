#include <iostream>

#if defined(__linux) || defined(__APPLE__)
// get process memory and total system memory for Linux and macOS
#include <libproc.h>
#include <sys/proc_info.h>
#include <unistd.h> // For getpid()
#endif

#if defined(__APPLE__)
// get total used system memory for macOS
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>
#endif

#include "MemoryUtilization.h"

namespace PokemonAutomation{



void print_macos_total_used_memory() {

}


MemoryUsage process_memory_usage(){
    MemoryUsage usage;

#if defined(__linux) || defined(__APPLE__)
    pid_t pid = getpid();
    struct proc_taskinfo task_info;
    int ret = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));

    if (ret <= 0) {
        std::cerr << "Error getting process info for PID " << pid << ": " << strerror(errno) << std::endl;
        return usage;
    }

    usage.process_physical_memory = task_info.pti_resident_size;
    usage.process_virtual_memory = task_info.pti_virtual_size;
    usage.total_system_memory = task_info.pti_total_system;

#if defined(__APPLE__) // compute total used system memory on macOS
    vm_size_t page_size;
    vm_statistics_data_t vm_stats;
    mach_port_t mach_port = mach_host_self();
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(integer_t);
    
    // Get the host statistics
    if (KERN_SUCCESS != host_statistics(mach_port, HOST_VM_INFO, (host_info_t)&vm_stats, &count)) {
        std::cerr << "Failed to get host statistics." << std::endl;
        return usage;
    }
    // Get the system's page size
    host_page_size(mach_port, &page_size);

    // Calculate used memory from vm_statistics
    // Used memory = Wired + Active + Inactive
    size_t wired_pages = vm_stats.wire_count;
    size_t active_pages = vm_stats.active_count;
    size_t inactive_pages = vm_stats.inactive_count;

    usage.total_used_system_memory = (wired_pages + active_pages + inactive_pages) * page_size;
#else  // compute total used system memory on Linux
    // TODO
#endif

#else // compute memory info on Windows
    // TODO
#endif
    return usage;
}


}