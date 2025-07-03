/*  Thread Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/ComputationThreadPool.h"
#include "ThreadUtilizationStats.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ThreadUtilizationStat::ThreadUtilizationStat(ThreadHandle handle, std::string label)
    : m_handle(handle)
    , m_label(std::move(label))
    , m_last_clock(thread_cpu_time(handle))
{}

OverlayStatSnapshot ThreadUtilizationStat::get_current(){
    std::lock_guard<std::mutex> lg(m_lock);

    WallClock now = current_time();
    WallDuration clock = thread_cpu_time(m_handle);
    if (clock == WallDuration::min()){
        return OverlayStatSnapshot{m_label + " ---"};
    }

    if (m_last_clock != WallDuration::min()){
        m_tracker.push_event(clock - m_last_clock, now);
    }
    m_last_clock = clock;

    return m_printer.get_snapshot(m_label, m_tracker.utilization());
}




ThreadPoolUtilizationStat::ThreadPoolUtilizationStat(const ComputationThreadPool& thread_pool, std::string label)
    : m_thread_pool(thread_pool)
    , m_label(std::move(label))
    , m_last_clock(thread_pool.cpu_time())
    , m_printer((double)thread_pool.max_threads())
{}

OverlayStatSnapshot ThreadPoolUtilizationStat::get_current(){
    std::lock_guard<std::mutex> lg(m_lock);

    WallClock now = current_time();
    WallDuration clock = m_thread_pool.cpu_time();
    if (clock <= WallDuration::zero()){
        return OverlayStatSnapshot();
    }

    if (m_last_clock != WallDuration::min()){
        m_tracker.push_event(clock - m_last_clock, now);
    }
    m_last_clock = clock;

    return m_printer.get_snapshot(
        m_label + " (x" + std::to_string(m_thread_pool.current_threads()) + "):",
        m_tracker.utilization()
    );
}





}
