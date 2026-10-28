/*  Spin Lock
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <thread>
#include <iostream>
#include "Common/Cpp/CpuId/CpuId.h"
#include "SpinPause.h"
#include "SpinLock.h"

#if 0
#elif defined _WIN32 && defined PA_ARCH_x86
#include <intrin.h>
uint64_t cpu_clock(){
    return __rdtsc();
}
#elif defined PA_ARCH_x86
uint64_t cpu_clock(){
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#elif defined PA_ARCH_arm64
//  The ARM equivalent of rdtsc: the generic timer's virtual counter. It can be
//  read in user space without entering the kernel.
uint64_t cpu_clock(){
    uint64_t value;
    __asm__ volatile ("mrs %0, cntvct_el0" : "=r" (value));
    return value;
}
#else
#define PA_NO_CPU_CLOCK
#endif

namespace PokemonAutomation{




void SpinLockMRSW::internal_acquire_read() noexcept{
//    cout << "SpinLockMRSW::internal_acquire_read()" << endl;

    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    while (true){
        pause();
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state != (size_t)-1 && m_readers.compare_exchange_weak(state, state + 1)){
            return;
        }
        if (++failed_iterations >= 1000){
            std::this_thread::yield();
            failed_iterations = 0;
        }
    }
}
void SpinLockMRSW::internal_acquire_write() noexcept{
//    cout << "SpinLockMRSW::internal_acquire_write()" << endl;

    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    while (true){
        pause();
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state == 0 && m_readers.compare_exchange_weak(state, (size_t)-1)){
            return;
        }
        if (++failed_iterations >= 1000){
            std::this_thread::yield();
            failed_iterations = 0;
        }
    }
}
void SpinLockMRSW::internal_acquire_read(const char* label) noexcept{
//    cout << "SpinLockMRSW::internal_acquire_read()" << endl;

#ifdef PA_NO_CPU_CLOCK
    internal_acquire_read();
#else
    uint64_t start = cpu_clock();
    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    while (true){
        pause();
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state != (size_t)-1 && m_readers.compare_exchange_weak(state, state + 1)){
            return;
        }
        if (cpu_clock() - start > 10000000000){
            try{
                std::cout << "Slow ReadSpinLock: " << label << std::endl;
            }catch (...){}
            start = cpu_clock();
        }
        if (++failed_iterations >= 1000){
            std::this_thread::yield();
            failed_iterations = 0;
        }
    }
#endif
}
void SpinLockMRSW::internal_acquire_write(const char* label) noexcept{
//    cout << "SpinLockMRSW::internal_acquire_write()" << endl;

#ifdef PA_NO_CPU_CLOCK
    internal_acquire_write();
#else
    uint64_t start = cpu_clock();
    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    while (true){
        pause();
        if (cpu_clock() - start > 10000000000){
            try{
                std::cout << "Slow WriteSpinLock: " << label << std::endl;
            }catch (...){}
            start = cpu_clock();
        }
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state == 0 && m_readers.compare_exchange_weak(state, (size_t)-1)){
            return;
        }
        if (++failed_iterations >= 1000){
            std::this_thread::yield();
            failed_iterations = 0;
        }
    }
#endif
}




}
