/*  Periodic Scheduler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PeriodicScheduler.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


size_t PeriodicScheduler::events() const{
    return m_events.size();
}
bool PeriodicScheduler::add_event(void* event, std::chrono::milliseconds period, WallClock start){
    auto ret = m_events.emplace(event, PeriodicEvent{m_callback_id, period});
    if (!ret.second){
        //  Already exists. Do nothing.
        return false;
    }

    //  Now add to schedule. Need to ensure strong exception safety.
    try{
        m_schedule.emplace(start, SingleEvent{m_callback_id, event});
    }catch (...){
        m_events.erase(ret.first);
        throw;
    }

    m_callback_id++;
    return true;
}
void PeriodicScheduler::remove_event(void* event){
    //  No need to remove from scheduler since it will be skipped over automatically.
    m_events.erase(event);
}
WallClock PeriodicScheduler::next_event() const{
    auto iter = m_schedule.begin();
    if (iter == m_schedule.end()){
        return WallClock::max();
    }
    return iter->first;
}
void* PeriodicScheduler::request_next_event(WallClock timestamp){
    while (true){
        auto iter0 = m_schedule.begin();

        //  Schedule is empty.
        if (iter0 == m_schedule.end()){
            return nullptr;
        }

        //  Next event isn't due.
        if (timestamp < iter0->first){
            return nullptr;
        }

        //  Current SingleEvent refers to a no longer existing PeriodicEvent.
        SingleEvent event = iter0->second;
        auto iter1 = m_events.find(event.event);
        if (iter1 == m_events.end() || event.id != iter1->second.id){
            m_schedule.erase(iter0);
            continue;
        }

        //  Schedule the next event first so that we retain strong exception safety if it throws.
        WallClock next = std::max(iter0->first + iter1->second.period, timestamp);
        m_schedule.emplace(next, iter0->second);

        //  Now remove the current event.
        m_schedule.erase(iter0);

        return event.event;
    }
}






#if 0
class ScopedCounter{
public:
    ScopedCounter(std::atomic<size_t>& counter)
        : m_counter(counter)
    {
        counter++;
    }
    ~ScopedCounter(){
        m_counter--;
    }

private:
    std::atomic<size_t>& m_counter;
};
#endif





PeriodicRunner::PeriodicRunner(AsyncDispatcher& dispatcher)
    : m_dispatcher(dispatcher)
    , m_pending_waits(0)
{}
bool PeriodicRunner::add_event(void* event, std::chrono::milliseconds period, WallClock start){
    throw_if_cancelled();

    m_pending_waits++;
    std::lock_guard<std::mutex> lg(m_lock);
    m_pending_waits--;

    //  Thread not started yet. Do this first for strong exception safety.
    if (!m_runner){
        m_runner = m_dispatcher.dispatch([this]{ thread_loop(); });
    }

    bool ret = m_scheduler.add_event(event, period, start);
    m_cv.notify_all();
    return ret;
}
void PeriodicRunner::remove_event(void* event){
    m_pending_waits++;
    std::lock_guard<std::mutex> lg(m_lock);
    m_pending_waits--;
    m_scheduler.remove_event(event);
    m_cv.notify_all();

    if (m_scheduler.events() == 0){
        WriteSpinLock lg1(m_stats_lock);
        m_utilization.push_idle();
    }
}
bool PeriodicRunner::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(std::move(exception))){
        return true;
    }
    std::lock_guard<std::mutex> lg(m_lock);
    m_cv.notify_all();
    return false;
}
void PeriodicRunner::thread_loop(){
    bool is_back_to_back = false;
    std::unique_lock<std::mutex> lg(m_lock);
    WallClock last_check_timestamp = current_time();
    WallDuration idle_since_last_check = WallDuration(0);
    while (true){
        if (cancelled()){
            return;
        }

        //  If anyone is trying to get the lock, yield now.
        if (m_pending_waits.load(std::memory_order_acquire) != 0){
            m_cv.wait(lg, [this]{ return m_pending_waits.load(std::memory_order_acquire) == 0; });
        }

        WallClock now = current_time();

        {
            WriteSpinLock lg1(m_stats_lock);
            m_utilization.push_event(now - last_check_timestamp - idle_since_last_check, now);
        }
        last_check_timestamp = now;
        idle_since_last_check = WallDuration(0);
//        cout << m_utilization.utilization() << endl;

        void* event = m_scheduler.request_next_event(now);

        //  Event is available now. Run it.
        if (event != nullptr){
            run(event, is_back_to_back);
            is_back_to_back = true;
            continue;
        }
        is_back_to_back = false;

        //  Wait for next scheduled event.
        WallClock next = m_scheduler.next_event();

        if (cancelled()){
            return;
        }

        WallClock start = current_time();
        if (next < WallClock::max()){
            m_cv.wait_until(lg, next);
        }else{
            m_cv.wait(lg);
        }
        WallClock end = current_time();
        idle_since_last_check += end - start;
    }
}
void PeriodicRunner::stop_thread(){
    PeriodicRunner::cancel(nullptr);
    m_runner.reset();
}

double PeriodicRunner::current_utilization() const{
    ReadSpinLock lg(m_stats_lock);
    return m_utilization.utilization();
}








}
