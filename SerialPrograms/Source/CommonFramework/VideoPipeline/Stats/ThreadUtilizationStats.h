/*  Thread Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ThreadUtilizationStats_H
#define PokemonAutomation_ThreadUtilizationStats_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/EventRateTracker.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/VideoPipeline/VideoOverlayTypes.h"

namespace PokemonAutomation{


class ThreadUtilizationStat : public OverlayStat{
public:
    ThreadUtilizationStat(ThreadHandle handle, std::string label);

    virtual OverlayStatSnapshot get_current() override;

private:
    ThreadHandle m_handle;
    std::string m_label;

    std::mutex m_lock;
    WallClock::duration m_last_clock;
    UtilizationTracker m_tracker;

    OverlayStatUtilizationPrinter m_printer;
};



}
#endif
