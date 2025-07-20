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



ComputationThreadPool& realtime_inference(){
    static ComputationThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_POOL.MAX_THREADS
    );
    return runner;
}
ComputationThreadPool& normal_inference(){
    static ComputationThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->NORMAL_THREAD_POOL.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, GlobalSettings::instance().PERFORMANCE->NORMAL_THREAD_POOL.MAX_THREADS
    );
    return runner;
}




}
}
