/*  Map weather & Time Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Read the weather and time symbols on the map.
 */


#ifndef PokemonAutomation_PokemonLA_MapWeatherAndTimeReader_H
#define PokemonAutomation_PokemonLA_MapWeatherAndTimeReader_H

#include "PokemonLA/PokemonLA_WeatherAndTime.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonLA{


Weather detect_weather_on_map(Logger& logger, const ImageViewRGB32& screen);


TimeOfDay detect_time_of_day_on_map(Logger& logger, const ImageViewRGB32& screen);



}
}
}
#endif
