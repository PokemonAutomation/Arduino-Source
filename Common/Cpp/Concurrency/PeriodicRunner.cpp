/*  Periodic Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "PeriodicRunner.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



PeriodicRunner::PeriodicRunner()
    : m_stopping(false)
    , m_thread(
        GlobalThreadPools::unlimited_normal().dispatch_now_blocking([this]{
            thread_body();
        })
    )
{}
PeriodicRunner::~PeriodicRunner(){
    stop();
}
void PeriodicRunner::stop(){
    if (!m_thread){
        return;
    }
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_stopping = true;
    }
    m_cv.notify_all();
    m_thread.wait_and_ignore_exceptions();
}


void PeriodicRunner::update_next(
    std::map<Runnable*, Entry>::iterator runnable_iter,
    std::multimap<WallClock, std::map<Runnable*, Entry>::iterator>::iterator schedule_iter,
    WallClock next_run_time
){
    runnable_iter->second.next = next_run_time;

    //  Move the node. This should never throw.
    auto node = m_schedule.extract(schedule_iter);
    node.key() = next_run_time;
    m_schedule.insert(std::move(node));
}


void PeriodicRunner::run_now_nonblocking(Runnable& runnable){
    {
        WallClock next = current_time();
        std::lock_guard<Mutex> lg(m_lock);

        auto runnable_iter = m_runnables.find(&runnable);
        if (runnable_iter == m_runnables.end()){
            return;
        }

        //  Already running right now!
        if (!runnable_iter->second.busy){
            return;
        }

        //  Find the corresponding schedule entry.
        auto schedule_iter = m_schedule.find(runnable_iter->second.next);
        while (true){
            if (schedule_iter == m_schedule.end()){
                //  Should be impossible.
                return;
            }
            if (schedule_iter->second->first == &runnable){
                break;
            }
            ++schedule_iter;
        }

        update_next(runnable_iter, schedule_iter, next);
    }
    m_cv.notify_all();
}
void PeriodicRunner::add_runnable(Runnable& runnable, WallDuration period){
    {
        std::lock_guard<Mutex> lg(m_lock);

        WallClock next = current_time();

        auto ret = m_runnables.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(&runnable),
            std::forward_as_tuple(period)
        );
        if (!ret.second){
            return;
        }

        try{
            m_schedule.emplace(next, ret.first);
        }catch (...){
            m_runnables.erase(ret.first);
            throw;
        }
    }
    m_cv.notify_all();
}
void PeriodicRunner::remove_runnable(Runnable& runnable) noexcept{
    while (true){
        std::lock_guard<Mutex> lg(m_lock);

        auto iter = m_runnables.find(&runnable);
        if (iter == m_runnables.end()){
            return;
        }

        if (!iter->second.busy){
            m_schedule.erase(iter->second.next);
            m_runnables.erase(iter);
            break;
        }
    }
}

void PeriodicRunner::thread_body(){
    std::unique_lock<Mutex> lg(m_lock);
    while (!m_stopping){
        auto schedule_iter = m_schedule.begin();
        if (schedule_iter == m_schedule.end()){
            m_cv.wait(lg);
            continue;
        }

        WallClock fire_time = schedule_iter->first;
        if (fire_time > current_time()){
            m_cv.wait_until(lg, fire_time);
            continue;
        }

        auto runnable_iter = schedule_iter->second;

        WallClock next = fire_time + runnable_iter->second.period;
        update_next(runnable_iter, schedule_iter, next);

        runnable_iter->second.busy = true;
        m_lock.unlock();
        runnable_iter->first->run();
        m_lock.lock();
        runnable_iter->second.busy = false;
    }
}






}
