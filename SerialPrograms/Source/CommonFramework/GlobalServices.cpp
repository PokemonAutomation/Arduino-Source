/*  Global Services
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "GlobalServices.h"

namespace PokemonAutomation{


AsyncDispatcher& global_async_dispatcher(){
    static AsyncDispatcher dispatcher(nullptr, 1);
    return dispatcher;
}
#if 0
ScheduledTaskRunner& global_scheduled_task_runner(){
    static ScheduledTaskRunner runner(global_async_dispatcher());
    return runner;
}
#endif
Watchdog& global_watchdog(){
    static Watchdog watchdog(GlobalThreadPools::unlimited_normal());
    return watchdog;
}



}
