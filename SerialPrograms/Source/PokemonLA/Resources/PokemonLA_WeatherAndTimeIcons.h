/*  Pokemon LA Weather And Time Icons
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_WeatherAndTimeIcons_H
#define PokemonAutomation_PokemonLA_WeatherAndTimeIcons_H

#include <array>
#include <string>
#include "PokemonLA/PokemonLA_WeatherAndTime.h"


namespace PokemonAutomation{

class ImageRGB32;

namespace NintendoSwitch{
namespace PokemonLA{

// weather icon images, in the order listed in PokemonLA_WeatherAndTime.h:Weather
const std::array<ImageRGB32, NUM_WEATHER>& ALL_WEATHER_ICONS();

// time of day icon images, in the order listed in PokemonLA_WeatherAndTime.h:TimeOfDay
// Note: TimeOfDay::None is not in the array.
const std::array<ImageRGB32, NUM_TIMES_OF_DAY>& ALL_TIME_OF_DAY_ICONS();

}
}
}
#endif
