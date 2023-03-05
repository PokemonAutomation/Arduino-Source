/*  Watchdog
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Watchdog.h"

namespace PokemonAutomation{



Watchdog::~Watchdog(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopped = true;
        m_cv.notify_all();
    }
    m_thread.join();
}
Watchdog::Watchdog()
    : m_thread(&Watchdog::thread_body, this)
{}

void Watchdog::update_unprotected(std::map<WatchdogCallback*, Entry>::iterator iter, WallClock next_call){
    if (next_call == iter->second.iter->first){
        //  No change in time.
        return;
    }

    //  Add the new time first. Then remove the old one. This ensures strong
    //  exception safety.
    auto new_iter = m_schedule.emplace(next_call, iter->first);
    m_schedule.erase(iter->second.iter);
    iter->second.iter = new_iter;
}

void Watchdog::add(WatchdogCallback& callback, std::chrono::milliseconds period){
    WallClock now = current_time();

    std::lock_guard<std::mutex> lg(m_lock);

    auto iter = m_callbacks.find(&callback);
    if (iter == m_callbacks.end()){
        //  Callback doesn't exist.

        auto iter1 = m_schedule.emplace(now + period, &callback);
        try{
            m_callbacks.emplace(&callback, Entry{period, iter1});
        }catch (...){
            m_schedule.erase(iter1);
            throw;
        }
    }else{
        //  Callback already exists.

        iter->second.period = period;
        update_unprotected(iter, now + period);
    }

    m_cv.notify_all();
}
void Watchdog::remove(WatchdogCallback& callback){
    std::lock_guard<std::mutex> lg(m_lock);
    auto iter = m_callbacks.find(&callback);
    if (iter == m_callbacks.end()){
        //  Callback doesn't exist.
        return;
    }

    m_schedule.erase(iter->second.iter);
    m_callbacks.erase(iter);
}


void Watchdog::delay(WatchdogCallback& callback){
    WallClock now = current_time();

    std::lock_guard<std::mutex> lg(m_lock);

    auto iter = m_callbacks.find(&callback);
    if (iter == m_callbacks.end()){
        //  Callback doesn't exist.
        return;
    }

    update_unprotected(iter, now + iter->second.period);
}
void Watchdog::delay(WatchdogCallback& callback, WallClock next_call){
    std::lock_guard<std::mutex> lg(m_lock);

    auto iter = m_callbacks.find(&callback);
    if (iter == m_callbacks.end()){
        //  Callback doesn't even exist.
        return;
    }

    update_unprotected(iter, next_call);
}
void Watchdog::delay(WatchdogCallback& callback, std::chrono::milliseconds delay){
    WallClock now = current_time();

    std::lock_guard<std::mutex> lg(m_lock);

    auto iter = m_callbacks.find(&callback);
    if (iter == m_callbacks.end()){
        //  Callback doesn't even exist.
        return;
    }

    update_unprotected(iter, now + delay);
}

void Watchdog::thread_body(){
    std::unique_lock<std::mutex> lg(m_lock);
    while (!m_stopped){
        if (m_schedule.empty()){
            //  Nothing scheduled.
            m_cv.wait(lg);
            continue;
        }

        auto iter_s = m_schedule.begin();
        auto iter_c = m_callbacks.find(iter_s->second);

        WallClock now = current_time();
        if (now < iter_s->first){
            //  Next scheduled callback isn't ready yet.
            m_cv.wait_until(lg, iter_s->first);
            continue;
        }

        iter_s->second->on_watchdog_timeout();

        update_unprotected(iter_c, current_time() + iter_c->second.period);
    }
}




}
