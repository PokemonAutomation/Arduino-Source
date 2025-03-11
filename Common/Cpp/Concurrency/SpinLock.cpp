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
#else
#define PA_NO_RDTSC
#endif


#include "SpinPause.h"
#include "SpinLock.h"

#include <iostream>
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



void SpinLockMRSW::internal_acquire_read(){
//    cout << "SpinLockMRSW::internal_acquire_read()" << endl;

    while (true){
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state != (size_t)-1 && m_readers.compare_exchange_weak(state, state + 1)){
            return;
        }
        pause();
    }
}
void SpinLockMRSW::internal_acquire_write(){
//    cout << "SpinLockMRSW::internal_acquire_write()" << endl;

    size_t state;
    do{
        pause();
        state = m_readers.load(std::memory_order_acquire);
    }while (state != 0 || !m_readers.compare_exchange_weak(state, (size_t)-1));
}
void SpinLockMRSW::internal_acquire_read(const char* label){
//    cout << "SpinLockMRSW::internal_acquire_read()" << endl;

#ifdef PA_NO_RDTSC
    internal_acquire_read();
#else
    uint64_t start = x86_rdtsc();
    while (true){
        size_t state = m_readers.load(std::memory_order_acquire);
        if (state != (size_t)-1 && m_readers.compare_exchange_weak(state, state + 1)){
            return;
        }
        pause();
        if (x86_rdtsc() - start > 10000000000){
            cout << "Slow ReadSpinLock: " << label << endl;
            start = x86_rdtsc();
        }
    }
#endif
}
void SpinLockMRSW::internal_acquire_write(const char* label){
//    cout << "SpinLockMRSW::internal_acquire_write()" << endl;

#ifdef PA_NO_RDTSC
    internal_acquire_write();
#else
    uint64_t start = x86_rdtsc();
    size_t state;
    do{
        pause();
        if (x86_rdtsc() - start > 10000000000){
            cout << "Slow WriteSpinLock: " << label << endl;
            start = x86_rdtsc();
        }
        state = m_readers.load(std::memory_order_acquire);
    }while (state != 0 || !m_readers.compare_exchange_weak(state, (size_t)-1));
#endif
}




}
