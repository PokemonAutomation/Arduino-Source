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



}
#endif
