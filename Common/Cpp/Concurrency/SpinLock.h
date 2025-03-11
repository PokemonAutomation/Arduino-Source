/*  Spin Lock
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_SpinLock_H
#define PokemonAutomation_SpinLock_H

#include <atomic>
#include "Common/Compiler.h"

namespace PokemonAutomation{


#if 0
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
#endif



//
//  This is a simple multi-reader/single-writer (MRSW) spin lock.
//  Fairness is not guaranteed. Multiple readers can starve a writer indefinitely.
//
//  As a spin lock, this is optimized for low contention.
//

class SpinLockMRSW{
public:
    SpinLockMRSW() : m_readers(0) {}

    PA_FORCE_INLINE void acquire_read(const char* label = nullptr){
        //  Optimization: Assume unlocked
        size_t state = 0;
        if (m_readers.compare_exchange_weak(state, 1)){
            return;
        }

        //  Only if we fail do we enter the (slow) retry logic.
        if (label == nullptr){
            internal_acquire_read();
        }else{
            internal_acquire_read(label);
        }
    }
    PA_FORCE_INLINE void acquire_write(const char* label = nullptr){
        //  Optimization: Assume unlocked
        size_t state = 0;
        if (m_readers.compare_exchange_weak(state, (size_t)-1)){
            return;
        }

        //  Only if we fail do we enter the (slow) retry logic.
        if (label == nullptr){
            internal_acquire_write();
        }else{
            internal_acquire_write(label);
        }
    }

    PA_FORCE_INLINE void unlock_read(){
        m_readers.fetch_sub(1);
    }
    PA_FORCE_INLINE void unlock_write(){
        m_readers.store(0, std::memory_order_release);
    }

private:
    void internal_acquire_read();
    void internal_acquire_write();
    void internal_acquire_read(const char* label);
    void internal_acquire_write(const char* label);

private:
    //  0           =   unlocked
    //  positive    =   # of readers holding lock
    //  -1          =   write locked
    std::atomic<size_t> m_readers;
};


class ReadSpinLock{
public:
    ReadSpinLock(const ReadSpinLock&) = delete;
    void operator=(const ReadSpinLock&) = delete;

    PA_FORCE_INLINE ReadSpinLock(SpinLockMRSW& lock, const char* label = "(unnamed lock)")
        : m_lock(lock)
    {
        lock.acquire_read(label);
    }
    PA_FORCE_INLINE ~ReadSpinLock(){
        m_lock.unlock_read();
    }

private:
    SpinLockMRSW& m_lock;
};


class WriteSpinLock{
public:
    WriteSpinLock(const WriteSpinLock&) = delete;
    void operator=(const WriteSpinLock&) = delete;

    PA_FORCE_INLINE WriteSpinLock(SpinLockMRSW& lock, const char* label = "(unnamed lock)")
        : m_lock(lock)
    {
        lock.acquire_write(label);
    }
    PA_FORCE_INLINE ~WriteSpinLock(){
        m_lock.unlock_write();
    }

private:
    SpinLockMRSW& m_lock;
};


using SpinLock = SpinLockMRSW;
using SpinLockGuard = WriteSpinLock;




}
#endif
