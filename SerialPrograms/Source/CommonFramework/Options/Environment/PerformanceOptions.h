/* Performance Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PerformanceOptions_H
#define PokemonAutomation_PerformanceOptions_H

#include "Common/Cpp/Options/GroupOption.h"
#include "ProcessPriorityOption.h"
#include "ProcessorLevelOption.h"

namespace PokemonAutomation{


class PerformanceOptions : public GroupOption{
public:
    PerformanceOptions()
        : GroupOption(
            "Performance",
            LockMode::LOCK_WHILE_RUNNING,
            GroupOption::EnableMode::ALWAYS_ENABLED, true
        )
        , REALTIME_THREAD_PRIORITY(
            "<b>Realtime Thread Priority:</b><br>"
            "Thread priority of real-time threads. (UI thread, audio threads)<br>"
            "Restart the program for this to fully take effect.",
            DEFAULT_PRIORITY_REALTIME
        )
        , INFERENCE_PRIORITY(
            "<b>Inference Priority:</b><br>"
            "Thread priority of inference threads. (image/sound recognition)",
            DEFAULT_PRIORITY_INFERENCE
        )
        , COMPUTE_PRIORITY(
            "<b>Compute Priority:</b><br>"
            "Thread priority of computation threads.",
            DEFAULT_PRIORITY_COMPUTE
        )
    {
        PA_ADD_OPTION(REALTIME_THREAD_PRIORITY);
        PA_ADD_OPTION(INFERENCE_PRIORITY);
        PA_ADD_OPTION(COMPUTE_PRIORITY);
        PA_ADD_OPTION(PROCESSOR_LEVEL);
    }

public:
    ThreadPriorityOption REALTIME_THREAD_PRIORITY;
    ThreadPriorityOption INFERENCE_PRIORITY;
    ThreadPriorityOption COMPUTE_PRIORITY;

    ProcessorLevelOption PROCESSOR_LEVEL;
};





}
#endif
