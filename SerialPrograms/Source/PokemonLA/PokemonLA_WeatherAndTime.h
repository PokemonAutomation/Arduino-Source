/*  Weather and Time of Day Definitions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_WeatherAndTime_H
#define PokemonAutomation_PokemonLA_WeatherAndTime_H



namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

enum class Weather {
    NONE = 0,
    SUNNY,
    CLOUDY,
    RAIN,
    SNOW,
    DROUGHT,
    FOG,
    RAINSTORM,
    SNOWSTORM,
};

// The order of `TimeOfDay` enums correpsonds to the order
// of setting the time in the camp, with `NONE` being no
// change of time whileing using the camp
enum class TimeOfDay {
    NONE = 0,
    MORNING,
    MIDDAY,
    EVENING,
    MIDNIGHT
};

// Map from int(TimeOfDay) to their char* names
extern const char* TIME_OF_DAY_NAMES[];

// Use one char to represent a time of day
char timeOfDayOneLetter(TimeOfDay time);

}
}
}
#endif
