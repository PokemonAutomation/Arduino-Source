/*  Hardware (arm64 Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Used for Linux aarch64 environment.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <thread>
#include "Hardware.h"

namespace PokemonAutomation{


// Linux: /proc/cpuinfo does not have a "Model name" line on ARM kernels,
// so fall back to the SoC "Hardware" line, and finally the kernel name.

static std::string cpuinfo_value(const std::string& file_path, const std::string& key){
    std::ifstream file(file_path);
    std::string line;
    while (std::getline(file, line)){
        if (line.rfind(key, 0) == 0){
            size_t pos = line.find(": ");
            if (pos == std::string::npos){
                return "";
            }
            return line.substr(pos + 2);
        }
    }
    return "";
}

uint64_t get_cpu_freq(){
    // /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq is in kHz.
    std::string max_freq = cpuinfo_value("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "");
    if (!max_freq.empty()){
        return (uint64_t)atoll(max_freq.c_str()) * 1000ULL;
    }

    // Fallback: "cpu MHz" in /proc/cpuinfo.
    std::string mhz = cpuinfo_value("/proc/cpuinfo", "cpu MHz");
    if (!mhz.empty()){
        return (uint64_t)(atof(mhz.c_str()) * 1000000.);
    }

    return 0;
}

std::string get_processor_name(){
    std::string name = cpuinfo_value("/proc/cpuinfo", "Model name");
    if (name.empty()){
        name = cpuinfo_value("/proc/cpuinfo", "Hardware");
    }
    if (name.empty()){
        name = "aarch64";
    }
    return name;
}


ProcessorSpecs get_processor_specs(){
    ProcessorSpecs specs;
    specs.name = get_processor_name();
    specs.base_frequency = get_cpu_freq();
    specs.threads = std::thread::hardware_concurrency();

    return specs;
}


}
