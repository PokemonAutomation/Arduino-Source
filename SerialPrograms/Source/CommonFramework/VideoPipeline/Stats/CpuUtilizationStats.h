/*  CPU Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CpuUtilizationStats_H
#define PokemonAutomation_CpuUtilizationStats_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/EventRateTracker.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/VideoPipeline/VideoOverlayTypes.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class CpuUtilizationStat : public OverlayStat{
public:
    CpuUtilizationStat();

    virtual OverlayStatSnapshot get_current() override;

private:
    std::mutex m_lock;
    SystemCpuTime m_last_clock;
    UtilizationTracker m_tracker;

    OverlayStatUtilizationPrinter m_printer;
};


inline CpuUtilizationStat::CpuUtilizationStat()
    : m_last_clock(SystemCpuTime::now())
{}
inline OverlayStatSnapshot CpuUtilizationStat::get_current(){
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
        m_tracker.push_event(std::chrono::duration_cast<WallClock::duration>(duration), now);
    }
    m_last_clock = current;

    return m_printer.get_snapshot("CPU Utilization (x" + std::to_string(vcores) + "):", m_tracker.utilization());
}




}
#endif
