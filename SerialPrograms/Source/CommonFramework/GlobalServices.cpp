/*  Global Services
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/Watchdog.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "GlobalServices.h"

namespace PokemonAutomation{


Watchdog& global_watchdog(){
    static Watchdog watchdog(GlobalThreadPools::unlimited_normal());
    return watchdog;
}



}
