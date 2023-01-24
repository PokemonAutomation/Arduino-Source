/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef _WIN32

#include <map>
#include <iostream>
#include <thread>
#include <Windows.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "Environment.h"
#include "Environment_Windows.h"

#if __GNUC__
#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif
#endif

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class WinApiHandleHolder{
public:
    WinApiHandleHolder(const WinApiHandleHolder& x) = delete;
    void operator=(const WinApiHandleHolder& x) = delete;
    ~WinApiHandleHolder(){
        CloseHandle(m_handle);
    }
    WinApiHandleHolder(HANDLE handle)
        : m_handle(handle)
    {}
    operator HANDLE() const{
        return m_handle;
    }

private:
    HANDLE m_handle;
};




const EnumDatabase<ThreadPriority>& PRIORITY_DATABASE(){
    static EnumDatabase<ThreadPriority> database({
        {ThreadPriority::Realtime,      "realtime",     "Realtime"},
        {ThreadPriority::High,          "high",         "High"},
        {ThreadPriority::AboveNormal,   "above-normal", "Above Normal"},
        {ThreadPriority::Normal,        "normal",       "Normal"},
        {ThreadPriority::BelowNormal,   "below-normal", "Below Normal"},
        {ThreadPriority::Low,           "low",          "Low"},
    });
    return database;
}

bool set_thread_priority(ThreadPriority priority){
    DWORD native_priority;
    switch (priority){
    case ThreadPriority::Realtime:
        native_priority = REALTIME_PRIORITY_CLASS;
        break;
    case ThreadPriority::High:
        native_priority = HIGH_PRIORITY_CLASS;
        break;
    case ThreadPriority::AboveNormal:
        native_priority = ABOVE_NORMAL_PRIORITY_CLASS;
        break;
    case ThreadPriority::Normal:
        native_priority = NORMAL_PRIORITY_CLASS;
        break;
    case ThreadPriority::BelowNormal:
        native_priority = BELOW_NORMAL_PRIORITY_CLASS;
        break;
    case ThreadPriority::Low:
        native_priority = IDLE_PRIORITY_CLASS;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority: " + std::to_string((int)priority));
    }
    if (SetPriorityClass(GetCurrentProcess(), native_priority)){
//        cout << "Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display << endl;
        global_logger_tagged().log("Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display, COLOR_BLUE);
        return true;
    }
    DWORD error = GetLastError();
    global_logger_tagged().log("Unable to set process priority. Error Code = " + std::to_string(error), COLOR_RED);
    return false;
}




ThreadHandle::ThreadHandle(ThreadHandle&& x) = default;
ThreadHandle& ThreadHandle::operator=(ThreadHandle&& x) = default;
ThreadHandle::ThreadHandle(const ThreadHandle& x) = default;
ThreadHandle& ThreadHandle::operator=(const ThreadHandle& x) = default;
ThreadHandle::~ThreadHandle() = default;
ThreadHandle::ThreadHandle(std::shared_ptr<WinApiHandleHolder> ptr)
    : m_ptr(std::move(ptr))
{}



ThreadHandle current_thread_handle(){
    DWORD id = GetCurrentThreadId();
    HANDLE handle = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, false, id);
//    cout << handle << endl;
    if (handle == NULL){
        return ThreadHandle();
    }
    return ThreadHandle{std::make_shared<WinApiHandleHolder>(handle)};
}
WallClock::duration thread_cpu_time(const ThreadHandle& handle){
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;

    if (!handle.m_ptr){
        return WallClock::duration::min();
    }

    if (!GetThreadTimes(*handle.m_ptr, &creation_time, &exit_time, &kernel_time, &user_time)){
        return WallClock::duration::min();
    }

    uint64_t nanos = user_time.dwLowDateTime + ((uint64_t)user_time.dwHighDateTime << 32);
    nanos *= 100;
    return std::chrono::duration_cast<WallClock::duration>(std::chrono::nanoseconds(nanos));
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
    FILETIME idle_time, kernel_time, user_time;
    if (GetSystemTimes(&idle_time, &kernel_time, &user_time)){
        m_idle = idle_time.dwLowDateTime + ((uint64_t)idle_time.dwHighDateTime << 32);
        m_kernel = kernel_time.dwLowDateTime + ((uint64_t)kernel_time.dwHighDateTime << 32);
        m_user = user_time.dwLowDateTime + ((uint64_t)user_time.dwHighDateTime << 32);
    }else{
        m_idle = 0;
        m_kernel = 0;
        m_user = 0;
    }
}
size_t SystemCpuTime::read_cores(){
    size_t cores = 0;
    WORD total_groups = GetActiveProcessorGroupCount();
    for (WORD group = 0; group < total_groups; group++){
        DWORD processors_within_group = GetActiveProcessorCount(group);
        cores += processors_within_group;
    }
    return cores;
}

















}
#endif
