/*  Global Thread Pools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/Backends/ThreadPool_Default.h"
#include "Common/Cpp/Hardware/FloatingPointMode.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "GlobalThreadPools.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{

//  Every pool thread turns on flush-to-zero for denormal floats when it starts.
//  Denormals are very slow on Intel CPUs, and image matching and ML inference
//  on these threads can produce them. See Common/Cpp/Hardware/FloatingPointMode.h.


ThreadPool& computation_realtime(){
    static ThreadPool_Default runner(
        [](){
            PerformanceOptions::instance().REALTIME_THREAD_POOL0.PRIORITY.set_on_this_thread(global_logger_tagged());
            enable_flush_denormals_to_zero();
        },
        0, PerformanceOptions::instance().REALTIME_THREAD_POOL0.MAX_THREADS
    );
    return runner;
}
ThreadPool& computation_normal(){
    static ThreadPool_Default runner(
        [](){
            PerformanceOptions::instance().NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
            enable_flush_denormals_to_zero();
        },
        0, PerformanceOptions::instance().NORMAL_THREAD_POOL.MAX_THREADS
    );
    return runner;
}

ThreadPool& unlimited_realtime(){
    static ThreadPool_Default runner(
        [](){
            PerformanceOptions::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread(global_logger_tagged());
            enable_flush_denormals_to_zero();
        },
        0
    );
    return runner;
}
ThreadPool& unlimited_pivot(){
    static ThreadPool_Default runner(
        [](){
            PerformanceOptions::instance().INFERENCE_PIVOT_PRIORITY0.set_on_this_thread(global_logger_tagged());
            enable_flush_denormals_to_zero();
        },
        0
    );
    return runner;
}
ThreadPool& unlimited_normal(){
    static ThreadPool_Default runner(
        [](){
            PerformanceOptions::instance().NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
            enable_flush_denormals_to_zero();
        },
        0
    );
    return runner;
}



}
}
