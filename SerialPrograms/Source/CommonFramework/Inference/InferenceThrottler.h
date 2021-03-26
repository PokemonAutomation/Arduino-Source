/*  Inference Throttler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceThrottler_H
#define PokemonAutomation_CommonFramework_InferenceThrottler_H

#include <chrono>
#include "CommonFramework/Tools/ProgramEnvironment.h"

namespace PokemonAutomation{

class InferenceThrottler{
public:
    InferenceThrottler(
        std::chrono::milliseconds timeout,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    )
        : m_timeout(timeout)
        , m_period(period)
        , m_start(std::chrono::system_clock::now())
        , m_wait_until(m_start + period)
    {}

    //  Call at the end of each loop iteration. This will wait until the
    //  time since the previous iteration has reached the specified period.
    //
    //  Returns true if this loop has timed out.
    bool end_iteration(ProgramEnvironment& env){
        auto now = std::chrono::system_clock::now();
        if (now - m_start >= m_timeout){
            return true;
        }

        auto wait = m_wait_until - now;
        if (wait <= std::chrono::milliseconds(0)){
            m_wait_until = now + m_period;
        }else{
            m_wait_until += m_period;
            env.wait(wait);
        }
        return false;
    }

    void set_period(std::chrono::milliseconds period){
        m_period = period;
    }

private:
    std::chrono::milliseconds m_timeout;
    std::chrono::milliseconds m_period;
    std::chrono::system_clock::time_point m_start;
    std::chrono::system_clock::time_point m_wait_until;
};



}
#endif
