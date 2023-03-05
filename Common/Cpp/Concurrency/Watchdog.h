/*  Watchdog
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This class runs callbacks at specified intervals. The user can also
 *  delay the next call.
 *
 *  The primary use case here is watchdog timers. If nothing happens within a
 *  time window, fire a callback to sound an alarm. But if something does
 *  happen, delay it since there's nothing wrong.
 *
 */

#ifndef PokemonAutomation_Watchdog_H
#define PokemonAutomation_Watchdog_H

#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{



struct WatchdogCallback{
    virtual void on_watchdog_timeout() = 0;
};


class Watchdog{
public:
    ~Watchdog();
    Watchdog();

    //  Add a callback which will be called at the specified period.
    //  If callback already exists, the period will be overwritten and the next
    //  call will be set to (now + period).
    void add(WatchdogCallback& callback, std::chrono::milliseconds period);
    void remove(WatchdogCallback& callback);

    //  Delay the specified callback.
    void delay(WatchdogCallback& callback);                                     //  Delay by the set period.
    void delay(WatchdogCallback& callback, WallClock next_call);                //  Delay to timestamp.
    void delay(WatchdogCallback& callback, std::chrono::milliseconds delay);    //  Delay by specific duration.

private:
    struct Entry{
        std::chrono::milliseconds period;
        std::multimap<WallClock, WatchdogCallback*>::iterator iter;
    };

    void update_unprotected(std::map<WatchdogCallback*, Entry>::iterator iter, WallClock next_call);

    void thread_body();

private:
    bool m_stopped = false;
    std::map<WatchdogCallback*, Entry> m_callbacks;
    std::multimap<WallClock, WatchdogCallback*> m_schedule;

    std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};





}
#endif
