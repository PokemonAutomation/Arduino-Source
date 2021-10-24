/*  Spin Lock
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_SpinLock_H
#define PokemonAutomation_SpinLock_H

#include <atomic>

namespace PokemonAutomation{


class SpinLock{
public:
    SpinLock() : m_locked(false) {}

    void spin_acquire();
    void spin_acquire(const char* label);

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

    SpinLockGuard(SpinLock& lock, const char* label = "(unnamed lock)")
        : m_lock(lock)
    {
        lock.spin_acquire(label);
    }
    ~SpinLockGuard(){
        m_lock.unlock();
    }

private:
    SpinLock& m_lock;
};


}
#endif
