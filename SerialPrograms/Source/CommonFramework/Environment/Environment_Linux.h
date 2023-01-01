/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Environment_Linux_H
#define PokemonAutomation_Environment_Linux_H

#include <chrono>
#include <pthread.h>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


enum class ThreadPriority{
    Max,
    Min,
};

constexpr ThreadPriority DEFAULT_PRIORITY_REALTIME  = ThreadPriority::Max;
constexpr ThreadPriority DEFAULT_PRIORITY_INFERENCE = ThreadPriority::Max;
constexpr ThreadPriority DEFAULT_PRIORITY_COMPUTE   = ThreadPriority::Min;



class ThreadHandle{
public:
    pthread_t handle;
};
ThreadHandle current_thread_handle();
WallClock::duration thread_cpu_time(const ThreadHandle& handle);



}
#endif
