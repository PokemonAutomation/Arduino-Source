/* Performance Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PerformanceOptions_H
#define PokemonAutomation_PerformanceOptions_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
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
        , PRECISE_WAKE_MARGIN(
            "<b>Precise Wake Time Margin:</b><br>"
            "Some operations require a thread to wake up at a very precise time - "
            "more precise than what the operating system's scheduler can provide. "
            "This option will force such operations to wake up this many "
            "microseconds earlier, then busy wait until the time is reached. "
            "The sys-botbase controller is an example of something that requires "
            "extremely precise wake times.",
            LockMode::UNLOCK_WHILE_RUNNING,
            "2000 us"
        )
        , REALTIME_THREAD_PRIORITY(
            "<b>Realtime Thread Priority:</b><br>"
            "Thread priority of real-time threads. (UI thread, audio threads)<br>"
            "Restart the program for this to fully take effect.",
            DEFAULT_PRIORITY_REALTIME
        )
        , REALTIME_INFERENCE_PRIORITY(
            "<b>Inference Priority:</b><br>"
            "Thread priority of realtime inference threads that must run fast "
            "enough to keep a program working properly.",
            DEFAULT_PRIORITY_REALTIME_INFERENCE
        )
        , NORMAL_INFERENCE_PRIORITY(
            "<b>normal Inference Priority:</b><br>"
            "Thread priority of non-realtime inference threads that can be slow "
            "without negatively affecting a program.",
            DEFAULT_PRIORITY_NORMAL_INFERENCE
        )
        , COMPUTE_PRIORITY(
            "<b>Compute Priority:</b><br>"
            "Thread priority of computation threads.",
            DEFAULT_PRIORITY_COMPUTE
        )
    {
        PA_ADD_OPTION(PRECISE_WAKE_MARGIN);
        PA_ADD_OPTION(REALTIME_THREAD_PRIORITY);
        PA_ADD_OPTION(REALTIME_INFERENCE_PRIORITY);
        PA_ADD_OPTION(NORMAL_INFERENCE_PRIORITY);
        PA_ADD_OPTION(COMPUTE_PRIORITY);
        PA_ADD_OPTION(PROCESSOR_LEVEL);
    }

public:
    MicrosecondsOption PRECISE_WAKE_MARGIN;

    ThreadPriorityOption REALTIME_THREAD_PRIORITY;
    ThreadPriorityOption REALTIME_INFERENCE_PRIORITY;
    ThreadPriorityOption NORMAL_INFERENCE_PRIORITY;
    ThreadPriorityOption COMPUTE_PRIORITY;

    ProcessorLevelOption PROCESSOR_LEVEL;
};





}
#endif
