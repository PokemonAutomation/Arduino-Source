/*  CPU Utilization (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <Windows.h>
#include "CpuUtilization_Windows.h"

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
WallDuration thread_cpu_time(const ThreadHandle& handle){
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;

    if (!handle.m_ptr){
        return WallDuration::min();
    }

    if (!GetThreadTimes(*handle.m_ptr, &creation_time, &exit_time, &kernel_time, &user_time)){
        return WallDuration::min();
    }

    uint64_t nanos = user_time.dwLowDateTime + ((uint64_t)user_time.dwHighDateTime << 32);
    nanos *= 100;
    return std::chrono::duration_cast<WallDuration>(std::chrono::nanoseconds(nanos));
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
