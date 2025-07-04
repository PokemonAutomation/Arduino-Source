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
        , INFERENCE_PIVOT_PRIORITY(
            "<b>Inference Pivot Priority:</b><br>"
            "Thread priority of inference dispatcher threads.",
            DEFAULT_PRIORITY_REALTIME_INFERENCE
        )
        , COMPUTE_PRIORITY(
            "<b>Compute Priority:</b><br>"
            "Thread priority of computation threads.",
            DEFAULT_PRIORITY_COMPUTE
        )
        , REALTIME_THREAD_POOL(
            "Real-time Thread Pool",
            "Thread pool for tasks that must run fast enough to keep a "
            "program running properly.<br>"
            "Restart program for changes to take full effect.",
            DEFAULT_PRIORITY_REALTIME_INFERENCE,
            0.5
        )
        , NORMAL_THREAD_POOL(
            "Normal Thread Pool",
            "Thread pool for tasks that can be slow without negatively "
            "affecting a program.<br>"
            "Restart program for changes to take full effect.",
            DEFAULT_PRIORITY_NORMAL_INFERENCE,
            1.0
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
    {
        PA_ADD_OPTION(PROCESSOR_LEVEL);

        PA_ADD_OPTION(REALTIME_THREAD_PRIORITY);
        PA_ADD_OPTION(INFERENCE_PIVOT_PRIORITY);
        PA_ADD_OPTION(COMPUTE_PRIORITY);

        PA_ADD_OPTION(REALTIME_THREAD_POOL);
        PA_ADD_OPTION(NORMAL_THREAD_POOL);

        PA_ADD_OPTION(PRECISE_WAKE_MARGIN);
    }

public:
    ProcessorLevelOption PROCESSOR_LEVEL;

    ThreadPriorityOption REALTIME_THREAD_PRIORITY;
    ThreadPriorityOption INFERENCE_PIVOT_PRIORITY;
    ThreadPriorityOption COMPUTE_PRIORITY;

    ThreadPoolOption REALTIME_THREAD_POOL;
    ThreadPoolOption NORMAL_THREAD_POOL;

    MicrosecondsOption PRECISE_WAKE_MARGIN;
};





}
#endif
