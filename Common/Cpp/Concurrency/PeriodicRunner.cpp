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

        if (iter->second.lock.try_lock()){
            m_schedule.erase(iter->second.next);
            m_runnables.erase(iter);
            break;
        }
    }
}

void PeriodicRunner::thread_body(){
    std::unique_lock<Mutex> lg(m_lock);
    while (!m_stopping){
        auto iter = m_schedule.begin();
        if (iter == m_schedule.end()){
            m_cv.wait(lg);
            continue;
        }

        WallClock fire_time = iter->first;
        if (fire_time > current_time()){
            m_cv.wait_until(lg, fire_time);
            continue;
        }

        auto runnable = iter->second;
        try{
            m_schedule.emplace(fire_time + runnable->second.period, runnable);
        }catch (...){
            continue;
        }

        m_schedule.erase(iter);

        std::lock_guard<Mutex> lg1(runnable->second.lock);
        m_lock.unlock();
        runnable->first->run();
        m_lock.lock();
    }
}






}
