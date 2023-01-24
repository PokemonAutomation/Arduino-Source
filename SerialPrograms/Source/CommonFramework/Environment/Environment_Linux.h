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
// Get the cpu time of the thread.
// This function must be called in the same process the thread belongs to.
WallClock::duration thread_cpu_time(const ThreadHandle& handle);



class SystemCpuTime{
public:
    static SystemCpuTime now();
    static size_t vcores();

    bool is_valid() const{
        return false;
    }

    std::chrono::microseconds operator-(const SystemCpuTime& x) const;

private:
    void set_to_now();
    static size_t read_cores();

private:
//    uint64_t m_time = 0;
};




}
#endif
