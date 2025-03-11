/*  Watchdog
 *
 *  From: https://github.com/PokemonAutomation/
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
#include "Common/Cpp/Concurrency/SpinLock.h"

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

    //  Delay the specified callback. If the callback is current running, this
    //  function does nothing.
    void delay(WatchdogCallback& callback);                                     //  Delay by the set period.
    void delay(WatchdogCallback& callback, WallClock next_call);                //  Delay to timestamp.
    void delay(WatchdogCallback& callback, std::chrono::milliseconds delay);    //  Delay by specific duration.

    //
    //  All public methods are fully thread-safe with each other. (except the destructor)
    //
    //  The above methods add() and delay(), are fast and will return quickly.
    //  So it is safe to call them in semi-performance critical places.
    //  (The critical section involves one spin-lock acquisition and 3 map lookups.)
    //
    //  remove() will also return quickly unless the callback being removed
    //  is currently running. In that case, it will block until it is done
    //  running.
    //

private:
    using Schedule = std::multimap<WallClock, WatchdogCallback&>;

    struct Entry{
        WatchdogCallback& callback;
        std::chrono::milliseconds period;
        Schedule::iterator iter;
        std::mutex lock;

        Entry(
            WatchdogCallback& p_callback,
            std::chrono::milliseconds p_period,
            Schedule::iterator p_iter
        )
            : callback(p_callback), period(p_period), iter(p_iter)
        {}
    };
    using CallbackMap = std::map<WatchdogCallback*, Entry>;

    //  Return true if the next call has moved up and we should signal.
    bool update_unprotected(CallbackMap::iterator iter, WallClock next_call);
    void thread_body();

private:
    bool m_stopped = false;
    CallbackMap m_callbacks;
    Schedule m_schedule;

    //  Nothing should ever acquire both locks at once.
    SpinLock m_state_lock;
    std::mutex m_sleep_lock;

    std::condition_variable m_cv;
    std::thread m_thread;
};





}
#endif
