/*  Recursive Throttler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Used to suppress recursive logging.
 *  Log only at the top level. And limit to 1 thread at a time.
 *
 */

#ifndef PokemonAutomation_RecursiveThrottler_H
#define PokemonAutomation_RecursiveThrottler_H

#include <mutex>

namespace PokemonAutomation{

class RecursiveThrottler{
public:
    //  Returns true if ok to run. (not throttled)
    operator bool() const{
        std::lock_guard<std::recursive_mutex> lg(m_lock);
        return m_depth == 0;
    }

private:
    friend class ThrottleScope;
    mutable std::recursive_mutex m_lock;
    size_t m_depth = 0;
};

class ThrottleScope{
    ThrottleScope(const ThrottleScope&) = delete;
    void operator=(const ThrottleScope&) = delete;
public:
    ThrottleScope(RecursiveThrottler& throttler)
        : m_throttler(throttler)
        , m_guard(throttler.m_lock)
    {
        throttler.m_depth++;
    }
    ~ThrottleScope(){
        m_throttler.m_depth--;
    }

    //  Returns true if ok to run. (not throttled)
    operator bool() const{
        return m_throttler.m_depth == 1;
    }

private:
    RecursiveThrottler& m_throttler;
    std::lock_guard<std::recursive_mutex> m_guard;
};

}
#endif
