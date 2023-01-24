/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Environment_Windows_H
#define PokemonAutomation_Environment_Windows_H

#include <chrono>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


enum class ThreadPriority{
    Realtime,
    High,
    AboveNormal,
    Normal,
    BelowNormal,
    Low,
};

constexpr ThreadPriority DEFAULT_PRIORITY_REALTIME  = ThreadPriority::High;
constexpr ThreadPriority DEFAULT_PRIORITY_INFERENCE = ThreadPriority::AboveNormal;
constexpr ThreadPriority DEFAULT_PRIORITY_COMPUTE   = ThreadPriority::BelowNormal;





class WinApiHandleHolder;

class ThreadHandle{
public:
    ThreadHandle(ThreadHandle&& x);
    ThreadHandle& operator=(ThreadHandle&& x);
    ThreadHandle(const ThreadHandle& x);
    ThreadHandle& operator=(const ThreadHandle& x);
    ~ThreadHandle();

    ThreadHandle(std::shared_ptr<WinApiHandleHolder> ptr = nullptr);

    std::shared_ptr<WinApiHandleHolder> m_ptr;
};
ThreadHandle current_thread_handle();
WallClock::duration thread_cpu_time(const ThreadHandle& handle);





class SystemCpuTime{
public:
    static SystemCpuTime now();
    static size_t vcores();

    bool is_valid() const{
        return m_kernel != 0;
    }

    std::chrono::microseconds operator-(const SystemCpuTime& x) const{
        uint64_t idle = m_idle - x.m_idle;
        uint64_t kernel = m_kernel - x.m_kernel;
        uint64_t user = m_user - x.m_user;
        uint64_t total_utilization = kernel + user - idle;
        return std::chrono::microseconds(total_utilization / 10);
    }

private:
    void set_to_now();
    static size_t read_cores();

private:
    uint64_t m_idle = 0;
    uint64_t m_kernel = 0;
    uint64_t m_user = 0;
};





}
#endif
