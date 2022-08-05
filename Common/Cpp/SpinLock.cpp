/*  Spin Lock
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CpuId/CpuId.h"

#if _WIN32 && defined PA_ARCH_x86
#include <intrin.h>
#endif

#include "SpinPause.h"
#include "SpinLock.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


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


}
