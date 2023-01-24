/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#if defined(__linux) || defined(__APPLE__)

#include <time.h>
#include <set>
#include <iostream>
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "Environment.h"

#if defined(__APPLE__)
#include <libproc.h>
#endif

// #include <iostream>


namespace PokemonAutomation{




const EnumDatabase<ThreadPriority>& PRIORITY_DATABASE(){
    static EnumDatabase<ThreadPriority> database({
        {ThreadPriority::Max, "max", "Max Priority"},
        {ThreadPriority::Min, "min", "Min Priority"},
    });
    return database;
}

bool set_thread_priority(ThreadPriority priority){
    int native_priority = sched_get_priority_min(SCHED_RR);
    switch (priority){
    case ThreadPriority::Max:
        native_priority = sched_get_priority_max(SCHED_RR);
        break;
    case ThreadPriority::Min:
        native_priority = sched_get_priority_min(SCHED_RR);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority Index: " + std::to_string((int)priority));
    }

    struct sched_param param;
    param.sched_priority = native_priority;
    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0){
        return true;
    }

    int errorcode = errno;
    global_logger_tagged().log("Unable to set process priority. Error Code = " + std::to_string(errorcode), COLOR_RED);
    return false;
}





ThreadHandle current_thread_handle(){
    return ThreadHandle{pthread_self()};
}
WallClock::duration thread_cpu_time(const ThreadHandle& handle){
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
        return WallClock::duration::min();
    }

#else
    clockid_t clockid;
    int error = pthread_getcpuclockid(handle.handle, &clockid);
    if (error){
        return WallClock::duration::min();
    }

    struct timespec ts;
    if (clock_gettime(clockid, &ts) == -1){
        return WallClock::duration::min();
    }

    nanos = (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif

    return std::chrono::duration_cast<WallClock::duration>(std::chrono::nanoseconds(nanos));
}






std::chrono::microseconds SystemCpuTime::operator-(const SystemCpuTime& x) const{
    return std::chrono::microseconds(0);
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
    //  TODO
}
size_t SystemCpuTime::read_cores(){
    return std::thread::hardware_concurrency();
}








}
#endif
