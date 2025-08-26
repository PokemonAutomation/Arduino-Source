/*  CPU Utilization Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuUtilizationStats_H
#define PokemonAutomation_CpuUtilizationStats_H

#include <mutex>
#include "Common/Cpp/EventRateTracker.h"
#include "Common/Cpp/CpuUtilization/CpuUtilization.h"
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










}
#endif
