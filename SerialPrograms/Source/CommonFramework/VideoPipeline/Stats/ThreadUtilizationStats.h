/*  Thread Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ThreadUtilizationStats_H
#define PokemonAutomation_ThreadUtilizationStats_H

#include <mutex>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/EventRateTracker.h"
#include "Common/Cpp/CpuUtilization/CpuUtilization.h"
#include "CommonFramework/VideoPipeline/VideoOverlayTypes.h"

namespace PokemonAutomation{


class ComputationThreadPool;



class ThreadUtilizationStat : public OverlayStat{
public:
    ThreadUtilizationStat(ThreadHandle handle, std::string label);

    virtual OverlayStatSnapshot get_current() override;

private:
    ThreadHandle m_handle;
    std::string m_label;

    std::mutex m_lock;
    WallDuration m_last_clock;
    UtilizationTracker m_tracker;

    OverlayStatUtilizationPrinter m_printer;
};


class ThreadPoolUtilizationStat : public OverlayStat{
public:
    ThreadPoolUtilizationStat(const ComputationThreadPool& thread_pool, std::string label);

    virtual OverlayStatSnapshot get_current() override;

private:
    const ComputationThreadPool& m_thread_pool;
    std::string m_label;

    std::mutex m_lock;
    WallDuration m_last_clock;
    UtilizationTracker m_tracker;

    OverlayStatUtilizationPrinter m_printer;
};




}
#endif
