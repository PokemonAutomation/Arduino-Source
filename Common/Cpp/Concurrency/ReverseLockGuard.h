/*  Reverse Lock Guard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ReverseLockGuard_H
#define PokemonAutomation_ReverseLockGuard_H

#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{


template <class Mutex>
class ReverseLockGuard{
public:
    ReverseLockGuard(Mutex& lock)
        : m_lock(lock)
    {
        if (lock.try_lock()){
            throw InternalProgramError(
                nullptr, PA_CURRENT_FUNCTION,
                "Attempted to unlock an already unlocked lock."
            );
        }
        lock.unlock();
    }
    ~ReverseLockGuard(){
        m_lock.lock();
    }


private:
    Mutex& m_lock;
};


}
#endif
