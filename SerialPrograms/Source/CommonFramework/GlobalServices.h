/*  Global Services
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GlobalServices_H
#define PokemonAutomation_GlobalServices_H

namespace PokemonAutomation{

class Watchdog;
class PeriodicRunner;


Watchdog& global_watchdog();
PeriodicRunner& global_periodic_runner();


}
#endif
