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
            GlobalSettings::instance().PERFORMANCE->THREAD_POOL_REALTIME_INFERENCE.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, GlobalSettings::instance().PERFORMANCE->THREAD_POOL_REALTIME_INFERENCE.MAX_THREADS
    );
    return runner;
}
ComputationThreadPool& normal_inference(){
    static ComputationThreadPool runner(
        [](){
            GlobalSettings::instance().PERFORMANCE->THREAD_POOL_NORMAL_INFERENCE.PRIORITY.set_on_this_thread(global_logger_tagged());
        },
        0, GlobalSettings::instance().PERFORMANCE->THREAD_POOL_NORMAL_INFERENCE.MAX_THREADS
    );
    return runner;
}




}
}
