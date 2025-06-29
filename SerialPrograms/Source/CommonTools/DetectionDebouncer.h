/*  Detection Debouncer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_DetectionDebouncer_H
#define PokemonAutomation_CommonTools_DetectionDebouncer_H

#include <functional>
#include <atomic>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


template <typename Type>
class DetectionDebouncer{
public:
    DetectionDebouncer(
        Type initial_state,
        std::chrono::milliseconds min_streak,
        std::function<void(Type)>&& state_reporter
    )
        : m_min_streak(min_streak)
        , m_state_reporter(std::move(state_reporter))
        , m_current_reported(initial_state)
        , m_last_match(current_time())
    {}

    //  This is fully thread-safe.
    Type get() const{
        return m_current_reported.load(std::memory_order_acquire);
    }

    //  Only one thread at a time is allowed to call this.
    Type push_value(Type value, WallClock timestamp){
        Type current_reported = get();

        //  No change.
        if (value == current_reported){
            m_last_match = timestamp;
            return current_reported;
        }

        //  Not long enough.
        if (m_last_match + m_min_streak > timestamp){
            return current_reported;
        }

        m_state_reporter(value);
        m_last_match = timestamp;
        m_current_reported.store(value, std::memory_order_release);
        return value;
    }


private:
    std::chrono::milliseconds m_min_streak;
    std::function<void(Type)> m_state_reporter;

    std::atomic<Type> m_current_reported;
    WallClock m_last_match;
};




}
#endif
