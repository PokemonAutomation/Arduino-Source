/*  Memory Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/MemoryUtilization/MemoryUtilization.h"
#include "MemoryUtilizationStats.h"

namespace PokemonAutomation{



OverlayStatSnapshot MemoryUtilizationStat::get_current(){
    if (m_parent){
        m_parent->update();
    }
    return m_snapshot;
}



void MemoryUtilizationStats::update(){
    MemoryUsage memory = process_memory_usage();

    double usage;

    constexpr uint64_t GB = (uint64_t)1 << 30;

    OverlayStatSnapshot system;
    if (get_stat(
        system.text, usage,
        "Sys: ",
        memory.total_system_memory,
        memory.total_used_system_memory
    )){
        uint64_t free_memory = memory.process_physical_memory - memory.process_virtual_memory;

        if (usage >= 0.90 || free_memory < 2*GB){
            system.color = COLOR_RED;
        }else if (usage >= 0.75 || free_memory < 4*GB){
            system.color = COLOR_ORANGE;
        }else if (usage >= 0.50 || free_memory < 8*GB){
            system.color = COLOR_YELLOW;
        }
    }

    OverlayStatSnapshot process;
    if (get_stat(
        process.text, usage,
        "App: ",
        memory.process_virtual_memory,
        memory.process_physical_memory
    )){
        if (memory.process_virtual_memory > 1*GB){
            if (usage < 0.10){
                process.color = COLOR_RED;
            }else if (usage < 0.25){
                process.color = COLOR_ORANGE;
            }else if (usage < 0.50){
                process.color = COLOR_YELLOW;
            }
        }
    }

    m_system.m_snapshot = std::move(system);
    m_process.m_snapshot = std::move(process);
}
bool MemoryUtilizationStats::get_stat(
    std::string& stat_text,
    double& usage,
    std::string label,
    uint64_t total, uint64_t current
){
    stat_text = std::move(label);
    stat_text += current != 0
        ? tostr_bytes(current)
        : "---";
    stat_text += " / ";
    stat_text += total != 0
        ? tostr_bytes(total)
        : "---";

    if (current != 0 && total != 0){
        usage = (double)current / total;
        stat_text += " (";
        stat_text += tostr_fixed(usage * 100, 1);
        stat_text += "%)";
        return true;
    }

    return false;
}




}
