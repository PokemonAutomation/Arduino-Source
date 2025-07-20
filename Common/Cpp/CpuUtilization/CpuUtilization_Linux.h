/*  CPU Utilization (Linux)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuUtilization_Linux_H
#define PokemonAutomation_CpuUtilization_Linux_H

#include <pthread.h>
#include <sys/time.h>
#include "Common/Cpp/Time.h"
//#include "CpuUtilization.h"

namespace PokemonAutomation{



class ThreadHandle{
public:
    pthread_t handle;
};
ThreadHandle current_thread_handle();
// Get the cpu time of the thread.
// This function must be called in the same process the thread belongs to.
WallDuration thread_cpu_time(const ThreadHandle& handle);



class SystemCpuTime{
public:
    static SystemCpuTime now();
    static size_t vcores();

    bool is_valid() const{
        return m_usec != 0 || m_sec != 0;
    }

    std::chrono::microseconds operator-(const SystemCpuTime& x) const;

private:
    void set_to_now();
    static size_t read_cores();

private:
    time_t m_sec = 0;
    suseconds_t m_usec = 0;
};




}
#endif
