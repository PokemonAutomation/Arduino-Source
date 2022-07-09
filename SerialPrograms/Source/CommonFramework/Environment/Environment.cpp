/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Environment.h"

#if _M_IX86 || _M_X64 || __i386__ || __x86_64__
#include "Environment_x86.tpp"
#endif

#if _WIN32
#include "Environment_Windows.tpp"
#ifdef PA_ARCH_x86
#include "Environment_x86_Windows.tpp"
#endif
#endif

#if defined(__linux) || defined(__APPLE__)
#include "Environment_Linux.tpp"
#ifdef PA_ARCH_x86
#include "Environment_x86_Linux.tpp"
#endif
#endif


namespace PokemonAutomation{



bool set_priority_by_name(const std::string& name){
    int index = priority_name_to_index(name);
    return set_priority_by_index(index);
}

int clip_priority(int priority){
    priority = std::min(priority, THREAD_PRIORITY_MAX);
    priority = std::max(priority, THREAD_PRIORITY_MIN);
    return priority;
}












}
