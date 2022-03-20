/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Environment_H
#define PokemonAutomation_Environment_H

#include <stdint.h>
#include <string>
#include <vector>
#include <QString>
#include <QThread>



namespace PokemonAutomation{



extern const int DEFAULT_PRIORITY_INDEX;
extern const std::vector<QString> PRIORITY_MODES;
int priority_name_to_index(const QString& name);
bool set_priority_by_index(int index);
bool set_priority_by_name(const QString& name);
int read_priority_index();


extern const int THREAD_PRIORITY_MIN;
extern const int THREAD_PRIORITY_MAX;
int clip_priority(int priority);
const char* thread_priority_name(int priority);
bool set_thread_priority(int priority);
QThread::Priority to_qt_priority(int priority);



uint64_t x86_rdtsc();

std::string get_processor_name();

uint64_t x86_measure_rdtsc_ticks_per_sec();
uint64_t x86_rdtsc_ticks_per_sec();


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
