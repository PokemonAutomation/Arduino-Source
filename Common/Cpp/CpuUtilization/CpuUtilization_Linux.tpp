/*  CPU Utilization (Linux)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <thread>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "CpuUtilization_Linux.h"

#if defined(__APPLE__)
#include <libproc.h>
#endif

namespace PokemonAutomation{



ThreadHandle current_thread_handle(){
    return ThreadHandle{pthread_self()};
}
WallDuration thread_cpu_time(const ThreadHandle& handle){
    uint64_t nanos = 0;

#if defined(__APPLE__)
    uint64_t tid = 0;
    pthread_threadid_np(handle.handle, &tid);
    struct proc_threadinfo pth;
    if (PROC_PIDTHREADINFO_SIZE ==
        proc_pidinfo(getpid(), PROC_PIDTHREADID64INFO, tid, &pth,
                    PROC_PIDTHREADINFO_SIZE)) {
        nanos = pth.pth_user_time + pth.pth_system_time; // user time + system time in ns
    }
    else{
        return WallDuration::min();
    }

#else
    clockid_t clockid;
    int error = pthread_getcpuclockid(handle.handle, &clockid);
    if (error){
        return WallDuration::min();
    }

    struct timespec ts;
    if (clock_gettime(clockid, &ts) == -1){
        return WallDuration::min();
    }

    nanos = (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif

    return std::chrono::duration_cast<WallDuration>(std::chrono::nanoseconds(nanos));
}






std::chrono::microseconds SystemCpuTime::operator-(const SystemCpuTime& x) const{
    return std::chrono::microseconds((m_sec - x.m_sec) * 1000000ull + (m_usec - x.m_usec));
}
SystemCpuTime SystemCpuTime::now(){
    SystemCpuTime ret;
    ret.set_to_now();
    return ret;
}
size_t SystemCpuTime::vcores(){
    static size_t cores = read_cores();
    return cores;
}
void SystemCpuTime::set_to_now(){
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) == -1){
        m_sec = 0;
        m_usec = 0;
    }else{
        m_sec = ru.ru_utime.tv_sec + ru.ru_stime.tv_sec;
        m_usec = ru.ru_utime.tv_usec + ru.ru_stime.tv_usec;
    }
}
size_t SystemCpuTime::read_cores(){
    return std::thread::hardware_concurrency();
}





}
