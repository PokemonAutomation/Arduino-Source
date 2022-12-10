/*  Scheduled Task Runner
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Run tasks at their scheduled times.
 *
 */

#include "ScheduledTaskRunner.h"

namespace PokemonAutomation{



ScheduledTaskRunner::~ScheduledTaskRunner(){
    ScheduledTaskRunner::cancel(nullptr);
    m_runner.reset();
}
ScheduledTaskRunner::ScheduledTaskRunner(AsyncDispatcher& dispatcher)
    : m_runner(dispatcher.dispatch([this]{ thread_loop(); }))
{}
size_t ScheduledTaskRunner::size() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_schedule.size();
}
void ScheduledTaskRunner::add_event(WallClock time, std::function<void()> callback){
    std::lock_guard<std::mutex> lg(m_lock);
    m_schedule.emplace(time, std::move(callback));
}
void ScheduledTaskRunner::add_event(std::chrono::milliseconds time_from_now, std::function<void()> callback){
    std::lock_guard<std::mutex> lg(m_lock);
    m_schedule.emplace(current_time() + time_from_now, std::move(callback));
}
bool ScheduledTaskRunner::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(std::move(exception))){
        return true;
    }
    std::lock_guard<std::mutex> lg(m_lock);
    m_cv.notify_all();
    return false;
}
void ScheduledTaskRunner::thread_loop(){
    std::unique_lock<std::mutex> lg(m_lock);
//    WallClock last_check_timestamp = current_time();
    while (true){
        if (cancelled()){
            return;
        }

        if (m_schedule.empty()){
            m_cv.wait(lg);
            continue;
        }

        auto item = m_schedule.begin();
        WallClock now = current_time();
        if (item->first > now){
            m_cv.wait_until(lg, item->first);
            continue;
        }
        lg.unlock();
        item->second();
        lg.lock();
        m_schedule.erase(item);
    }
}





}
