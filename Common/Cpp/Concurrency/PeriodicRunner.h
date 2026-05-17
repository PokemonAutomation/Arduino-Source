/*  Periodic Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PeriodicRunner_H
#define PokemonAutomation_PeriodicRunner_H

#include <map>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"

namespace PokemonAutomation{



class PeriodicRunner{
public:
    struct Runnable{
        virtual void run() noexcept = 0;
    };


public:
    PeriodicRunner();
    ~PeriodicRunner();

    void stop();

    void add_runnable(Runnable& runnable, WallDuration period);
    void remove_runnable(Runnable& runnable) noexcept;


private:
    void thread_body();


private:
    struct Entry{
        Mutex lock;
        WallDuration period;
        WallClock next;

        Entry(WallDuration p_period)
            : period(p_period)
            , next(current_time())
        {}
    };

    Mutex m_lock;
    ConditionVariable m_cv;

    std::map<Runnable*, Entry> m_runnables;
    std::multimap<WallClock, std::map<Runnable*, Entry>::iterator> m_schedule;

    bool m_stopping;
    AsyncTask m_thread;
};




}
#endif
