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
#include "Common/Cpp/CpuId/CpuId.h"

namespace PokemonAutomation{



extern const int DEFAULT_PRIORITY_INDEX;
extern const std::vector<std::string> PRIORITY_MODES;
int priority_name_to_index(const std::string& name);
bool set_priority_by_index(int index);
bool set_priority_by_name(const std::string& name);
int read_priority_index();


extern const int THREAD_PRIORITY_MIN;
extern const int THREAD_PRIORITY_MAX;
int clip_priority(int priority);
const char* thread_priority_name(int priority);
bool set_thread_priority(int priority);
QThread::Priority to_qt_priority(int priority);



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
