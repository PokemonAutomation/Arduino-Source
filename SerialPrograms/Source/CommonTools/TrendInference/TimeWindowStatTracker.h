/*  Time Window Stat Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_TimeWindowStatTracker_H
#define PokemonAutomation_CommonTools_TimeWindowStatTracker_H

#include <chrono>
#include <map>

namespace PokemonAutomation{


template <typename StatAccumulator>
class TimeWindowStatTracker{
    using milliseconds = std::chrono::milliseconds;
    using system_clock = std::chrono::system_clock;
    using StatObject = typename StatAccumulator::StatObject;

public:
    TimeWindowStatTracker(milliseconds window)
        : m_window(window)
    {}

    milliseconds window() const{
        return m_window;
    }

    const StatObject& oldest() const{
        return m_history.begin()->second;
    }
    const StatObject& newest() const{
        return m_history.rbegin()->second;
    }

    system_clock::time_point push(
        const StatObject& stats,
        system_clock::time_point timestamp = system_clock::now()
    ){
        m_history.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(timestamp),
            std::forward_as_tuple(stats)
        );

        system_clock::time_point tail = timestamp - m_window;
        while (!m_history.empty() && m_history.begin()->first < tail){
            m_history.erase(m_history.begin());
        }

        return timestamp;
    }
    system_clock::time_point push(
        StatObject&& stats,
        system_clock::time_point timestamp = system_clock::now()
    ){
        m_history.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(timestamp),
            std::forward_as_tuple(std::move(stats))
        );

        system_clock::time_point tail = timestamp - m_window;
        while (!m_history.empty() && m_history.begin()->first < tail){
            m_history.erase(m_history.begin());
        }

        return timestamp;
    }

    //  Accumulate entire history.
    StatAccumulator accumulate_all() const{
        return accumulate(m_history.begin(), m_history.end());
    }

    //  Accumulate [latest - duration, latest].
    StatAccumulator accumulate_last(
        milliseconds duration
    ) const{
        return accumulate(
            m_history.empty()
                ? m_history.end()
                : m_history.lower_bound(m_history.rbegin()->first - duration),
            m_history.end()
        );
    }

    //  Accumulate [oldest, point].
    StatAccumulator accumulate_start_to_point(
        system_clock::time_point point
    ) const{
        return accumulate(m_history.begin, m_history.upper_bound(point));
    }

    //  Accumulate [oldest, latest - time_behind_latest].
    StatAccumulator accumulate_start_to_point(
        milliseconds time_behind_latest
    ) const{
        return accumulate(
            m_history.begin(),
            m_history.empty()
                ? m_history.end()
                : m_history.upper_bound(m_history.rbegin()->first - time_behind_latest)
        );
    }

    //  Accumulate [start, end].
    StatAccumulator accumulate(
        system_clock::time_point start,
        system_clock::time_point end
    ) const{
        return accumulate(m_history.lower_bound(start), m_history.upper_bound(end));
    }


private:
    using const_iterator_type = typename std::map<system_clock::time_point, StatObject>::const_iterator;

    StatAccumulator accumulate(
        const_iterator_type start,
        const_iterator_type end
    ) const{
        StatAccumulator accumulator;
        for (; start != end; ++start){
            accumulator += start->second;
        }
        return accumulator;
    }


private:
    milliseconds m_window;
    std::map<system_clock::time_point, StatObject> m_history;

};



}
#endif
