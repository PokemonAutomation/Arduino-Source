/*  Keyboard State Tracker
 *
 *  From: https://github.com/PokemonAutomation/
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
    KeyboardStateTracker(std::chrono::milliseconds debounce_period = std::chrono::milliseconds(10));


public:
    void clear();
    void press(uint32_t key);
    void release(uint32_t key);


public:
    WallClock next_state_change() const;
    std::set<uint32_t> get_currently_pressed();


private:
    //  Move all pending events that are old enough to the committed set.
    void move_old_events_unprotected(WallClock now);


private:
    struct Event{
        WallClock timestamp;
        bool press;
        uint32_t key;
    };
    const std::chrono::milliseconds m_debounce_period;

    std::set<uint32_t> m_committed;
    std::deque<Event> m_pending;
};







}
#endif
