/*  Global Thread Pools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/Backends/ThreadPool_Default.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "GlobalThreadPools.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{

//  The pools are deliberately leaked (never deleted): a pool's destructor
//  joins all of its worker threads, and a join at process exit hangs
//  without any diagnostics when one worker is still inside a task that
//  never finishes (the Qt 6.9 thread-adoption bug made this far worse -
//  see #570: "process hangs after returning from main()"). Normal mode
//  joins explicitly at the end of "main()" (the "Force stop the thread
//  pools" block in Main.cpp), which this does not affect. Command-line
//  test mode exits without joining at all: the threads live until the
//  process exits and the OS reclaims them, the same exit treatment the
//  workaround platforms already get.

ThreadPool& computation_realtime(){
    static ThreadPool_Default* runner = new ThreadPool_Default(
        [](){
            PerformanceOptions::instance().REALTIME_THREAD_POOL0.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, PerformanceOptions::instance().REALTIME_THREAD_POOL0.MAX_THREADS
    );
    return *runner;
}
ThreadPool& computation_normal(){
    static ThreadPool_Default* runner = new ThreadPool_Default(
        [](){
            PerformanceOptions::instance().NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, PerformanceOptions::instance().NORMAL_THREAD_POOL.MAX_THREADS
    );
    return *runner;
}

ThreadPool& unlimited_realtime(){
    static ThreadPool_Default* runner = new ThreadPool_Default(
        [](){
            PerformanceOptions::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread(global_logger_tagged());
        },
        0
    );
    return *runner;
}
ThreadPool& unlimited_pivot(){
    static ThreadPool_Default* runner = new ThreadPool_Default(
        [](){
            PerformanceOptions::instance().INFERENCE_PIVOT_PRIORITY0.set_on_this_thread(global_logger_tagged());
        },
        0
    );
    return *runner;
}
ThreadPool& unlimited_normal(){
    static ThreadPool_Default* runner = new ThreadPool_Default(
        [](){
            PerformanceOptions::instance().NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0
    );
    return *runner;
}



}
}
