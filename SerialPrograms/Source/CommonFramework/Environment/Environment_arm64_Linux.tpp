/*  Environment (arm64 Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Currently only used for M-series Apple environment
 */


#include <stdio.h>
#include <stdint.h>
#include <sys/sysctl.h>

#include "Environment.h"

namespace PokemonAutomation{


uint64_t get_cpu_freq()
{
    uint64_t freq = 0;
    size_t size = sizeof(freq);

    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
    return freq;
}

std::string get_processor_name(){
    char name_buffer[100] = "";
    size_t size = 100;
    if (sysctlbyname("machdep.cpu.brand_string", name_buffer, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
    return name_buffer;
}


ProcessorSpecs get_processor_specs(){
    ProcessorSpecs specs;
    specs.name = get_processor_name();
    specs.base_frequency = get_cpu_freq();
    specs.threads = std::thread::hardware_concurrency();

    return specs;
}


}
