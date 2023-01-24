/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Environment_H
#define PokemonAutomation_Environment_H

#include <string>
#include <vector>
#include <QThread>
#include "Common/Cpp/EnumDatabase.h"
#include "Common/Cpp/CpuId/CpuId.h"

#if _WIN32
#include "Environment_Windows.h"
#elif defined(__linux) || defined(__APPLE__)
#include "Environment_Linux.h"
#else
#error "Unsupported platform."
#endif

namespace PokemonAutomation{


const EnumDatabase<ThreadPriority>& PRIORITY_DATABASE();

bool set_thread_priority(ThreadPriority priority);


class ThreadHandle;



std::string get_processor_name();
struct ProcessorSpecs{
    std::string name;
    size_t threads = 0;
    size_t cores = 0;
    size_t sockets = 0;
    size_t numa_nodes = 0;
    size_t base_frequency = 0;
};
ProcessorSpecs get_processor_specs();









}
#endif
