/*  Global Services
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GlobalServices_H
#define PokemonAutomation_GlobalServices_H

namespace PokemonAutomation{

class AsyncDispatcher;
class ScheduledTaskRunner;
class Watchdog;


AsyncDispatcher& global_async_dispatcher();
//ScheduledTaskRunner& global_scheduled_task_runner();
Watchdog& global_watchdog();



}
#endif
