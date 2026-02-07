/*  Global Thread Pools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "GlobalThreadPools.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{



ThreadPool& computation_realtime(){
    static ThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_POOL.MAX_THREADS
    );
    return runner;
}
ThreadPool& computation_normal(){
    static ThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, GlobalSettings::instance().PERFORMANCE->NORMAL_THREAD_POOL.MAX_THREADS
    );
    return runner;
}

ThreadPool& unlimited_realtime(){
    static ThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0
    );
    return runner;
}
ThreadPool& unlimited_pivot(){
    static ThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->INFERENCE_PIVOT_PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0
    );
    return runner;
}
ThreadPool& unlimited_normal(){
    static ThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0
    );
    return runner;
}



}
}
