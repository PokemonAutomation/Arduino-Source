/*  Keyboard State Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "KeyboardStateTracker.h"

namespace PokemonAutomation{



KeyboardStateTracker::KeyboardStateTracker(std::chrono::milliseconds debounce_period)
    : m_debounce_period(debounce_period)
{}



void KeyboardStateTracker::clear(){
    m_committed.clear();
    m_pending.clear();
}
void KeyboardStateTracker::press(uint32_t key){
    WallClock now = current_time();
    m_pending.emplace_back(Event{now, true, key});
    move_old_events_unprotected(now);
}
void KeyboardStateTracker::release(uint32_t key){
    WallClock now = current_time();
    m_pending.emplace_back(Event{now, false, key});
    move_old_events_unprotected(now);
}



WallClock KeyboardStateTracker::next_state_change() const{
    return m_pending.empty()
        ? WallClock::max()
        : m_pending.begin()->timestamp + m_debounce_period;
}
std::set<uint32_t> KeyboardStateTracker::get_currently_pressed(){
    WallClock now = current_time();
    move_old_events_unprotected(now);

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
void KeyboardStateTracker::move_old_events_unprotected(WallClock now){
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





}
