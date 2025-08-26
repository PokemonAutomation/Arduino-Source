/*  CPU Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Time.h"
#include "CpuUtilizationStats.h"

namespace PokemonAutomation{


CpuUtilizationStat::CpuUtilizationStat()
    : m_last_clock(SystemCpuTime::now())
{}
OverlayStatSnapshot CpuUtilizationStat::get_current(){
    std::lock_guard<std::mutex> lg(m_lock);

    WallClock now = current_time();
    SystemCpuTime current = SystemCpuTime::now();
    size_t vcores = SystemCpuTime::vcores();
    if (vcores == 0 || !current.is_valid()){
        return OverlayStatSnapshot{"CPU Utilization: ---"};
    }


    if (m_last_clock.is_valid()){
        auto duration = current - m_last_clock;
        duration /= vcores;
        m_tracker.push_event(std::chrono::duration_cast<WallDuration>(duration), now);
    }
    m_last_clock = current;

    return m_printer.get_snapshot("CPU Utilization (x" + std::to_string(vcores) + "):", m_tracker.utilization());
}


}
