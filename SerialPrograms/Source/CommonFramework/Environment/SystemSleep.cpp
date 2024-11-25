/*  OS Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "SystemSleep.h"

#if 0
#elif _WIN32
#include "SystemSleep_Windows.tpp"
#elif __APPLE__
#include "SystemSleep_Apple.tpp"
#else
namespace PokemonAutomation{
    SystemSleepController& SystemSleepController::instance(){
        static SystemSleepController controller;
        return controller;
    }
}
#endif

namespace PokemonAutomation{






}

