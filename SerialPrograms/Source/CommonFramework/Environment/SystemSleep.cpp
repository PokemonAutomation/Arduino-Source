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
    SystemSleepController(){}
    ~SystemSleepController(){}
    void push_screen_on(){}
    void pop_screen_on(){}
    void push_no_sleep(){}
    void pop_no_sleep(){}
}
#endif

namespace PokemonAutomation{






}

