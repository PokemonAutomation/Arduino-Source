/*  Spin Lock
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#ifndef PokemonAutomation_SpinLock_H
#define PokemonAutomation_SpinLock_H

#include <atomic>
#include <emmintrin.h>
#include <intrin.h>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


class SpinLock{
public:
    SpinLock() : m_locked(false) {}

    void spin_acquire(){
        while (true){
            bool state = false;
            if (!m_locked.load(std::memory_order_acquire) && m_locked.compare_exchange_weak(state, true)){
                break;
            }
            _mm_pause();
        }
    }
    void spin_acquire(const char* label){
        uint64_t start = __rdtsc();
        while (true){
            bool state = false;
            if (!m_locked.load(std::memory_order_acquire) && m_locked.compare_exchange_weak(state, true)){
                break;
            }
            _mm_pause();

            if (__rdtsc() - start > 10000000000){
                cout << "Slow SpinLock: " << label << endl;
                start = __rdtsc();
            }
        }
    }

    void unlock(){
        m_locked.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> m_locked;
};


class SpinLockGuard{
public:
    SpinLockGuard(const SpinLockGuard&) = delete;
    void operator=(const SpinLockGuard&) = delete;

    SpinLockGuard(SpinLock& lock, const char* label)
        : m_lock(lock)
        , m_label(label)
    {
        lock.spin_acquire(label);
    }
    ~SpinLockGuard(){
        m_lock.unlock();
    }

private:
    SpinLock& m_lock;
    const char* m_label;
};


}
#endif
