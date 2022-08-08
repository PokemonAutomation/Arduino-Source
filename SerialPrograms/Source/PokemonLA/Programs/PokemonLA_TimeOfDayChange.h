/*  Time of Day Change
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TimeOfDayChange_H
#define PokemonAutomation_PokemonLA_TimeOfDayChange_H

#include "PokemonLA/PokemonLA_WeatherAndTime.h"
#include "PokemonLA/PokemonLA_Locations.h"


#include <stdint.h>

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonLA{


void change_time_of_day_at_tent(ConsoleHandle& console, BotBaseContext& context, TimeOfDay target_time, Camp camp);



}
}
}
#endif
