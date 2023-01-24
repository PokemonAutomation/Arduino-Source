/*  Thread Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
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
    WallClock::duration clock = thread_cpu_time(m_handle);
    if (clock == WallClock::duration::min()){
        return OverlayStatSnapshot{m_label + " ---"};
    }

    if (m_last_clock != WallClock::duration::min()){
        m_tracker.push_event(clock - m_last_clock, now);
    }
    m_last_clock = clock;

    return m_printer.get_snapshot(m_label, m_tracker.utilization());
}



}
