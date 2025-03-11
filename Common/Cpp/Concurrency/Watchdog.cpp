/*  Watchdog
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "Watchdog.h"

//using std::cout;
//using std::endl;

namespace PokemonAutomation{



Watchdog::~Watchdog(){
    {
        std::lock_guard<std::mutex> lg(m_sleep_lock);
        m_stopped = true;
        m_cv.notify_all();
    }
    m_thread.join();
}
Watchdog::Watchdog()
    : m_thread(&Watchdog::thread_body, this)
{}


void Watchdog::add(WatchdogCallback& callback, std::chrono::milliseconds period){
    bool signal = true;

    WallClock now = current_time();
    {
        WriteSpinLock slg(m_state_lock);

        auto iter = m_callbacks.find(&callback);
        if (iter == m_callbacks.end()){
            //  Callback doesn't exist. Add it.
            auto iter1 = m_schedule.emplace(now + period, callback);
            try{
                m_callbacks.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(&callback),
                    std::forward_as_tuple(callback, period, iter1)
                );
            }catch (...){
                m_schedule.erase(iter1);
                throw;
            }
        }else{
            //  Callback already exists. Update the period.
            iter->second.period = period;
            signal = update_unprotected(iter, now + period);
        }
    }

    if (signal){
        std::lock_guard<std::mutex> wlg(m_sleep_lock);
        m_cv.notify_all();
    }
}
void Watchdog::remove(WatchdogCallback& callback){
    while (true){
        {
            WriteSpinLock slg(m_state_lock);
            auto iter_c = m_callbacks.find(&callback);
            if (iter_c == m_callbacks.end()){
                return;
            }

            std::mutex& entry_lock = iter_c->second.lock;
            if (entry_lock.try_lock()){
                //  Remove from the schedule first.
                auto iter_s = iter_c->second.iter;
                m_schedule.erase(iter_s);

                //  Unlock and remove from callback set.
                entry_lock.unlock();
                m_callbacks.erase(iter_c);
                return;
            }
        }
        pause();
    }
}



bool Watchdog::update_unprotected(CallbackMap::iterator iter, WallClock next_call){
    //  Must be called under the "m_state_lock".

    if (next_call == iter->second.iter->first){
        //  No change in time.
        return false;
    }

    WallClock current_wake_time = m_schedule.begin()->first;

    //  Add the new time first. Then remove the old one. This ensures strong
    //  exception safety.
    auto new_iter = m_schedule.emplace(next_call, *iter->first);
    m_schedule.erase(iter->second.iter);
    iter->second.iter = new_iter;

    WallClock new_wake_time = m_schedule.begin()->first;
    return new_wake_time < current_wake_time;
}
void Watchdog::delay(WatchdogCallback& callback, WallClock next_call){
    bool signal = false;

    WallClock now = current_time();
    {
        WriteSpinLock slg(m_state_lock);
        auto iter = m_callbacks.find(&callback);
        if (iter == m_callbacks.end()){
            return;
        }
        signal = update_unprotected(
            iter,
            next_call == WallClock::min()
                ? now + iter->second.period
                : next_call
        );
    }

    if (signal){
        std::lock_guard<std::mutex> wlg(m_sleep_lock);
        m_cv.notify_all();
    }
}
void Watchdog::delay(WatchdogCallback& callback){
    this->delay(callback, WallClock::min());
}
void Watchdog::delay(WatchdogCallback& callback, std::chrono::milliseconds delay){
    this->delay(callback, current_time() + delay);
}


void Watchdog::thread_body(){
    WallClock wake_time = WallClock::min();
    while (true){
        {
            std::unique_lock<std::mutex> wlg(m_sleep_lock);
            if (m_stopped){
                break;
            }
            if (wake_time == WallClock::max()){
//                cout << "Sleeping indefinitely..." << endl;
                m_cv.wait(wlg);
//                cout << "Waking up..." << endl;
            }else if (wake_time != WallClock::min()){
//                cout << "Sleeping until... " << std::chrono::duration_cast<std::chrono::milliseconds>(wake_time - current_time()) << endl;
                m_cv.wait_until(wlg, wake_time);
//                cout << "Waking up..." << endl;
            }
        }
//        cout << "Checking..." << endl;

        WallClock now = current_time();

        std::unique_lock<std::mutex> elg;
        Entry* entry;
        CallbackMap::iterator iter_c;
        {
            WriteSpinLock slg(m_state_lock);

            //  Nothing scheduled.
            if (m_schedule.empty()){
//                cout << "Nothing scheduled..." << endl;
                wake_time = WallClock::max();
                continue;
            }

            //  Check if the next scheduled thing is ready to run.

            Schedule::iterator iter_s = m_schedule.begin();
            iter_c = m_callbacks.find(&iter_s->second);
            if (now < iter_s->first){
                //  Not ready to run yet.
//                cout << "Not ready to run yet..." << endl;
                wake_time = iter_s->first;
                continue;
            }else{
                //  Ready to run.
                entry = &iter_c->second;
                elg = std::unique_lock<std::mutex>(entry->lock);
            }
        }
//        cout << "Running..." << endl;

        //  Run the callback.
        try{
            entry->callback.on_watchdog_timeout();
        }catch (Exception& e){
            std::cerr << "Watchdog callback threw an exception: " << e.to_str() << std::endl;
        }catch (std::exception& e){
            std::cerr << "Watchdog callback threw an exception: " << e.what() << std::endl;
        }catch (...){
            std::cerr << "Watchdog callback threw an exception." << std::endl;
        }

        WriteSpinLock slg(m_state_lock);
        update_unprotected(iter_c, current_time() + entry->period);
//        entry->lock.unlock();

        wake_time = WallClock::min();
    }
}




}
