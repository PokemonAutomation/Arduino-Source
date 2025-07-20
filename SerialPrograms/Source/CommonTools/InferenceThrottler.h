/*  Inference Throttler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_InferenceThrottler_H
#define PokemonAutomation_CommonTools_InferenceThrottler_H

#include <chrono>
#include "Common/Cpp/CancellableScope.h"

namespace PokemonAutomation{

class InferenceThrottler{
public:
    InferenceThrottler(
        std::chrono::milliseconds timeout,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    )
        : m_timeout(timeout)
        , m_period(period)
        , m_start(current_time())
        , m_wait_until(m_start + period)
    {}

    //  Call at the end of each loop iteration. This will wait until the
    //  time since the previous iteration has reached the specified period.
    //
    //  Returns true if this loop has timed out.
    bool end_iteration(CancellableScope& scope){
        auto now = current_time();
        if (m_timeout != std::chrono::milliseconds(0) && now - m_start >= m_timeout){
            return true;
        }

        std::chrono::milliseconds wait = std::chrono::duration_cast<std::chrono::milliseconds>(m_wait_until - now);
        if (wait <= std::chrono::milliseconds(0)){
            m_wait_until = now + m_period;
        }else{
            m_wait_until += m_period;
            scope.wait_for(wait);
        }
        return false;
    }

    void set_period(std::chrono::milliseconds period){
        m_period = period;
    }

private:
    std::chrono::milliseconds m_timeout;
    std::chrono::milliseconds m_period;
    WallClock m_start;
    WallClock m_wait_until;
};



}
#endif
