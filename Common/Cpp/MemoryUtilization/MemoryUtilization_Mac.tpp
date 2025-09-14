/*  Memory Utilization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MemoryUtilization_Mac_TPP
#define PokemonAutomation_MemoryUtilization_Mac_TPP

#include <iostream>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/mach_vm.h>
#include <mach/vm_statistics.h>

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
    static int64_t physical_memory = 0;
    if (physical_memory == 0) {
        int mib[] = {CTL_HW, HW_MEMSIZE};
        size_t length = sizeof(physical_memory);
        if (sysctl(mib, 2, &physical_memory, &length, NULL, 0) != 0) {
            std::cerr << "Error calling sysctl()." << std::endl;
        }
    }
    usage.total_system_memory = physical_memory;
    {
        task_vm_info_data_t tvi;
        mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
        if(KERN_SUCCESS == task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) &tvi, &count))
        {
            usage.process_physical_memory = tvi.resident_size; // resident_size = internal + external + reusable
        } else {
            std::cerr << "Failed to get task info." << std::endl;
        }
    }
    {
        mach_vm_size_t size = 0;
        vm_region_extended_info_data_t info;
        mach_msg_type_number_t count = VM_REGION_EXTENDED_INFO_COUNT;
        mach_port_t object_name;
        unsigned int pages_swapped_out = 0;

        for (mach_vm_address_t address = 0;; address += size) {
            auto kr = mach_vm_region(mach_task_self(), &address, &size, VM_REGION_EXTENDED_INFO, (vm_region_info_t)&info, &count, &object_name);
            if (kr != KERN_SUCCESS) {
                if (kr == KERN_INVALID_ADDRESS) { // end of the address space
                    break;
                }
                std::cerr << "mach_vm_region failed with error: " << mach_error_string(kr) << std::endl;
                break;
            }
            pages_swapped_out += info.pages_swapped_out;
            address += size;
        }
        usage.process_virtual_memory = usage.process_physical_memory + (size_t)pages_swapped_out * (size_t)vm_page_size;
    }
    // ref: https://github.com/htop-dev/htop/blob/main/darwin/Platform.c
    #ifdef __arm64__
    {
        vm_statistics64_data_t vm_stats;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        if (KERN_SUCCESS == host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info_t)&vm_stats, &count)) {
            auto used = vm_stats.active_count + vm_stats.inactive_count +
              vm_stats.speculative_count + vm_stats.wire_count + vm_stats.compressor_page_count
              - vm_stats.purgeable_count - vm_stats.external_page_count;
            usage.total_used_system_memory = (size_t)used * (size_t)vm_page_size;
        } else {
            std::cerr << "Failed to get host statistics64." << std::endl;
        }
    }
    #else
    {
        vm_statistics_data_t vm_stats;
        mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
        if (KERN_SUCCESS == host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stats, &count)) {
            usage.total_used_system_memory = (size_t)(vm_stats.active_count + vm_stats.wire_count) * (size_t)vm_page_size;
        } else {
            std::cerr << "Failed to get host statistics." << std::endl;
        }
    }
    #endif
    return usage;
}




}
#endif
