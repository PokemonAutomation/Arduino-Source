/*  Stop Watch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Stopwatch_H
#define PokemonAutomation_VideoPipeline_Stopwatch_H

#include "Time.h"

namespace PokemonAutomation{


class Stopwatch{
public:
    Stopwatch()
        : m_total(WallDuration::zero())
        , m_last_start(WallClock::max())
    {}

    void start(){
//        cout << "start" << endl;
        if (m_last_start != WallClock::max()){
            return;
        }
        m_last_start = current_time();
    }
    void stop(){
//        cout << "stop" << endl;
        if (m_last_start == WallClock::max()){
            return;
        }
        m_total += current_time() - m_last_start;
        m_last_start = WallClock::max();
    }
    WallDuration total() const{
        WallDuration ret = m_total;
        if (m_last_start != WallClock::max()){
            ret += current_time() - m_last_start;
        }
//        cout << "total: " << std::chrono::duration_cast<Milliseconds>(ret).count() << endl;
        return ret;
    }

private:
    WallDuration m_total;
    WallClock m_last_start;
};



}
#endif
