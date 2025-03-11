/*  System Sleep
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SystemSleep_H
#define PokemonAutomation_SystemSleep_H

#include <set>
#include <atomic>
#include <mutex>
#include "Common/Cpp/ListenerSet.h"

#if _WIN32
#define PA_ENABLE_SLEEP_SUPPRESS
#define PA_ENABLE_SLEEP_SUPPRESS_NO_SLEEP
#endif
#if __APPLE__
#define PA_ENABLE_SLEEP_SUPPRESS
#endif


namespace PokemonAutomation{



enum class SleepSuppress{
    NONE,
    NO_SLEEP,
    SCREEN_ON,
};


//  Call OS API to prevent screen saver from running and OS from going to sleep.
//  Useful for running some programs like PokemonSV_VideoFastCodeEntry that require
//  the screen to be constantly on.
class SystemSleepController{
public:
    struct Listener{
        virtual void sleep_suppress_state_changed(SleepSuppress new_state) = 0;
    };
    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


protected:
    virtual ~SystemSleepController() = default;
    SystemSleepController();

    //  Must be called under lock.
    void notify_listeners(SleepSuppress state);


public:
    static SystemSleepController& instance();

    SleepSuppress current_state() const;

    //  Push: Add a request for this type of sleep-disable.
    //  Pop: Remove a request for this type of sleep-disable.
    //  The sleep-disable will be active as long as there is at least one
    //  request is active for that type.

    //  Keep the screen on and prevent sleep.
    virtual void push_screen_on(){}
    virtual void pop_screen_on(){}

    //  Allow the screen to turn off, but don't sleep.
    virtual void push_no_sleep(){}
    virtual void pop_no_sleep(){}


protected:
    std::atomic<SleepSuppress> m_state;

    std::mutex m_lock;
    ListenerSet<Listener> m_listeners;
};





class SleepSuppressScope{
public:
    SleepSuppressScope(SleepSuppress mode){
        set(mode);
    }
    ~SleepSuppressScope(){
        clear();
    }
    SleepSuppressScope(const SleepSuppressScope&) = delete;
    void operator=(const SleepSuppressScope&) = delete;

    void clear();
    void operator=(SleepSuppress mode);

private:
    void set(SleepSuppress mode);

private:
    SleepSuppress m_mode;
};




}
#endif
