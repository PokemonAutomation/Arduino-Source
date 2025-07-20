/*  Event Rate Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_EventRateTracker_H
#define PokemonAutomation_EventRateTracker_H

#include <deque>
#include "Time.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


//  A class for calculating stuff like frames/second.
class EventRateTracker{
public:
    EventRateTracker(std::chrono::milliseconds window = std::chrono::milliseconds(1000))
        : m_window(window)
    {}

    void push_event(WallClock timestamp = current_time()){
        WallClock threshold = timestamp - m_window;
        while (!m_history.empty()){
            if (m_history.front() >= threshold){
                break;
            }
            m_history.pop_front();
        }
        m_history.emplace_back(timestamp);
    }

    size_t events_in_window() const{
        return m_history.size();
    }
    double events_per_second() const{
        if (m_history.size() < 2){
            return 0;
        }
        if (current_time() > m_history.back() + m_window){
            return 0;
        }
        auto delta = m_history.back() - m_history.front();
        return (double)(m_history.size() - 1) / std::chrono::duration_cast<std::chrono::microseconds>(delta).count() * 1000000;
    }

private:
    std::chrono::milliseconds m_window;
    std::deque<WallClock> m_history;
};



class UtilizationTracker{
public:
    UtilizationTracker(WallDuration window = std::chrono::milliseconds(1000))
        : m_window(window)
        , m_min_window(window / 2)
        , m_running_usage(0)
    {}

    void push_event(WallDuration usage_since_last, WallClock timestamp = current_time()){
        WallClock threshold = timestamp - m_window;
        while (m_history.size() > 1){
            if (m_history.front().timestamp >= threshold){
                break;
            }
            m_running_usage -= m_history.front().usage;
            m_history.pop_front();
        }
        m_history.emplace_back(Entry{timestamp, usage_since_last});
        m_running_usage += usage_since_last;
    }
    void push_idle(){
        m_history.clear();
        m_running_usage = WallDuration(0);
    }

    size_t events_in_window() const{
        return m_history.size();
    }
    WallDuration usage_in_window() const{
        return m_running_usage;
    }
    double utilization() const{
        if (m_history.size() < 2){
            return 0;
        }
        auto iter = m_history.begin();
        auto delta_time = m_history.back().timestamp - iter->timestamp;
        delta_time = std::max(delta_time, m_min_window);
        auto usage = m_running_usage - iter->usage;
        std::chrono::microseconds time_usec = std::chrono::duration_cast<std::chrono::microseconds>(delta_time);
        if (time_usec.count() == 0){
            return 0;
        }
        std::chrono::microseconds usage_usec = std::chrono::duration_cast<std::chrono::microseconds>(usage);
        return (double)usage_usec.count() / time_usec.count();
    }


private:
    struct Entry{
        WallClock timestamp;
        WallDuration usage;
    };

    WallDuration m_window;
    WallDuration m_min_window;

    std::deque<Entry> m_history;
    WallDuration m_running_usage;
};




}
#endif
