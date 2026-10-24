/*  Spin Lock
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"


#if 0
#elif defined _WIN32 && defined PA_ARCH_x86
#include <intrin.h>
uint64_t x86_rdtsc(){
    return __rdtsc();
}
#elif defined PA_ARCH_x86
uint64_t x86_rdtsc(){
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#elif defined PA_ARCH_arm64
//  The ARM equivalent of rdtsc: the generic timer's virtual counter. It can be
//  read in user space without entering the kernel.
uint64_t x86_rdtsc(){
    uint64_t value;
    __asm__ volatile ("mrs %0, cntvct_el0" : "=r" (value));
    return value;
}
#else
#define PA_NO_RDTSC
#endif


#include "SpinPause.h"
#include "SpinLock.h"

#include <iostream>
#include <thread>
using std::cout;
using std::endl;

namespace PokemonAutomation{




#if 0
void SpinLock::spin_acquire(){
    bool state = false;
    if (m_locked.compare_exchange_weak(state, true)){
        return;
    }
    do{
        pause();
        state = false;
    }while (m_locked.load(std::memory_order_acquire) || !m_locked.compare_exchange_weak(state, true));
}

void SpinLock::spin_acquire(const char* label){
#if _WIN32 && defined PA_ARCH_x86
    bool state = false;
    if (m_locked.compare_exchange_weak(state, true)){
//        cout << "early" << endl;
//        printf("early\n");
        return;
    }
    uint64_t start = __rdtsc();
    do{
//        cout << "late" << endl;
//        printf("late\n");
        pause();
        if (__rdtsc() - start > 10000000000){
            cout << "Slow SpinLock: " << label << endl;
            start = __rdtsc();
        }
        state = false;
    }while (m_locked.load(std::memory_order_acquire) || !m_locked.compare_exchange_weak(state, true));
#else
    spin_acquire();
#endif
}
#endif



void SpinLockMRSW::internal_acquire_read() noexcept{
//    cout << "SpinLockMRSW::internal_acquire_read()" << endl;

    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    while (true){
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state != (size_t)-1 && m_readers.compare_exchange_weak(state, state + 1)){
            return;
        }
        pause();
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
    size_t state;
    while (true){
        pause();
        state = m_readers.load(std::memory_order_acquire);
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

#ifdef PA_NO_RDTSC
    internal_acquire_read();
#else
    uint64_t start = x86_rdtsc();
    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    while (true){
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state != (size_t)-1 && m_readers.compare_exchange_weak(state, state + 1)){
            return;
        }
        pause();
        if (x86_rdtsc() - start > 10000000000){
            try{
                cout << "Slow ReadSpinLock: " << label << endl;
            }catch (...){}
            start = x86_rdtsc();
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

#ifdef PA_NO_RDTSC
    internal_acquire_write();
#else
    uint64_t start = x86_rdtsc();
    //  Yield to the OS every 1000 failed iterations (but not immediately).
    size_t failed_iterations = 0;
    size_t state;
    while (true){
        pause();
        if (x86_rdtsc() - start > 10000000000){
            try{
                cout << "Slow WriteSpinLock: " << label << endl;
            }catch (...){}
            start = x86_rdtsc();
        }
        state = m_readers.load(std::memory_order_acquire);
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
