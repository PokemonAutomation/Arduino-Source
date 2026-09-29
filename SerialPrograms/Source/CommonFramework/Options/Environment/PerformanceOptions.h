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
#include "OnnxOptions.h"

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

    ThreadPriorityOption UI_THREAD_PRIORITY;
    ThreadPriorityOption REALTIME_THREAD_PRIORITY0;
    ThreadPriorityOption INFERENCE_PIVOT_PRIORITY0;
    ThreadPriorityOption COMPUTE_PRIORITY;

    ThreadPoolOption REALTIME_THREAD_POOL0;
    ThreadPoolOption NORMAL_THREAD_POOL;

    MicrosecondsOption PRECISE_WAKE_MARGIN;

    OnnxOptions ONNX_OPTIONS;
};





}
#endif
