/* Performance Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PerformanceOptions.h"

namespace PokemonAutomation{



PerformanceOptions::PerformanceOptions()
    : GroupOption(
        "Performance",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED,
        true,
        false
    )
    , UI_THREAD_PRIORITY(
        "<b>UI Thread Priority:</b><br>"
        "Thread priority of the UI thread. This affects the responsiveness of the UI under load.<br>"
        "Restart the program for this to fully take effect.",
        DEFAULT_PRIORITY_UI
    )
    , REALTIME_THREAD_PRIORITY0(
        "<b>Realtime Thread Priority:</b><br>"
        "Thread priority of real-time threads. (program/controller threads)<br>"
        "Restart the program for this to fully take effect.",
        DEFAULT_PRIORITY_REALTIME
    )
    , INFERENCE_PIVOT_PRIORITY0(
        "<b>Inference Pivot Priority:</b><br>"
        "Thread priority of inference dispatcher threads.",
        DEFAULT_PRIORITY_REALTIME_INFERENCE
    )
    , COMPUTE_PRIORITY(
        "<b>Compute Priority:</b><br>"
        "Thread priority of computation threads.",
        DEFAULT_PRIORITY_COMPUTE
    )
    , REALTIME_THREAD_POOL0(
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
#ifdef _WIN32
    PA_ADD_OPTION(CORE_AFFINITY);
#endif

    PA_ADD_OPTION(UI_THREAD_PRIORITY);
    PA_ADD_OPTION(REALTIME_THREAD_PRIORITY0);
    PA_ADD_OPTION(INFERENCE_PIVOT_PRIORITY0);
    PA_ADD_OPTION(COMPUTE_PRIORITY);

    PA_ADD_OPTION(REALTIME_THREAD_POOL0);
    PA_ADD_OPTION(NORMAL_THREAD_POOL);

    //  Used only by sys-botbase 2 which has been removed.
//    PA_ADD_OPTION(PRECISE_WAKE_MARGIN);

    PA_ADD_OPTION(ONNX_OPTIONS);
}



}
