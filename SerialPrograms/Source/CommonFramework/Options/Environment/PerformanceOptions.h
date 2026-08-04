/* Performance Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PerformanceOptions_H
#define PokemonAutomation_PerformanceOptions_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Options/ThreadPoolOption.h"
#include "ProcessPriorityOption.h"
#include "ProcessorLevelOption.h"
#include "CoreAffinityOption.h"

namespace PokemonAutomation{


class PerformanceOptions : public GroupOption{
public:
    static PerformanceOptions& instance(){
        static PerformanceOptions option;
        return option;
    }

private:
    PerformanceOptions();

public:
    ProcessorLevelOption PROCESSOR_LEVEL;
#ifdef _WIN32
    CoreAffinityOption CORE_AFFINITY;
#endif

    ThreadPriorityOption REALTIME_THREAD_PRIORITY;
    ThreadPriorityOption INFERENCE_PIVOT_PRIORITY;
    ThreadPriorityOption COMPUTE_PRIORITY;

    ThreadPoolOption REALTIME_THREAD_POOL;
    ThreadPoolOption NORMAL_THREAD_POOL;

    MicrosecondsOption PRECISE_WAKE_MARGIN;
};





}
#endif
