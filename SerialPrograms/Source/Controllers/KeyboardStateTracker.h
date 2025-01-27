/*  Keyboard State Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_KeyboardStateTracker_H
#define PokemonAutomation_Controllers_KeyboardStateTracker_H

#include <deque>
#include <set>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{



class KeyboardStateTracker{
public:
    KeyboardStateTracker(std::chrono::milliseconds debounce_period = std::chrono::milliseconds(10))
        : m_debounce_period(debounce_period)
    {}


public:
    void clear(){
        m_committed.clear();
        m_pending.clear();
    }
    void press(uint32_t key){
        WallClock now = current_time();
        m_pending.emplace_back(Event{now, true, key});
        move_old_events(now);
    }
    void release(uint32_t key){
        WallClock now = current_time();
        m_pending.emplace_back(Event{now, false, key});
        move_old_events(now);
    }


public:
    WallClock next_state_change() const{
        return m_pending.empty()
            ? WallClock::max()
            : m_pending.begin()->timestamp + m_debounce_period;
    }
    std::set<uint32_t> get_currently_pressed(){
        WallClock now = current_time();
        move_old_events(now);

        //  Copy the current committed set.
        std::set<uint32_t> ret = m_committed;

        //  Now process all the pending ones - ignoring the release events.
        for (const Event& event : m_pending){
            if (event.press){
                ret.insert(event.key);
            }
        }

        return ret;
    }


private:
    //  Move all pending events that are old enough to the committed set.
    void move_old_events(WallClock now){
        WallClock threshold = now - m_debounce_period;
        while (!m_pending.empty()){
            Event& event = m_pending.front();

            //  Always commit presses.
            if (event.press){
                m_committed.insert(event.key);
                m_pending.pop_front();
                continue;
            }

            //  Release is old enough. Commit it.
            if (event.timestamp < threshold){
                m_committed.erase(event.key);
                m_pending.pop_front();
                continue;
            }

            //  We've hit an uncommittable release. We're done.
            break;
        }
    }


private:
    struct Event{
        WallClock timestamp;
        bool press;
        uint32_t key;
    };
    std::chrono::milliseconds m_debounce_period;
    std::set<uint32_t> m_committed;
    std::deque<Event> m_pending;
};



}
#endif
