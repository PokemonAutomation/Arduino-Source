/*  Weather and Time of Day Definitions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_WeatherAndTime_H
#define PokemonAutomation_PokemonLA_WeatherAndTime_H

#include <array>
#include <string>
#include <stdint.h>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

enum class Weather{
    SUNNY = 0,
    CLOUDY,
    RAIN,
    SNOW,
    DROUGHT,
    FOG,
    RAINSTORM,
    SNOWSTORM,
};

constexpr size_t NUM_WEATHER = 8;

// Map from int(Weather) to their names
// "Sunny",
// "Cloudy",
// "Rain",
// "Snowy",
// "Drought",
// "Fog",
// "Rainstorm",
// "Snowstorm"
extern const std::array<std::string, NUM_WEATHER> WEATHER_NAMES;

// From weather name (listed in WEATHER_NAMES) to enum
Weather get_weather(const std::string& name);

// The order of `TimeOfDay` enums corresponds to the order
// of setting the time in the camp, with `NONE` being no
// change of time while using the camp
enum class TimeOfDay{
    NONE = 0,
    MORNING,
    MIDDAY,
    EVENING,
    MIDNIGHT
};

// Number of time setting options listed in enum TimeOfDay
constexpr size_t NUM_TIME_CHANGE_CHOICES = 5;
// Number of unique times
constexpr size_t NUM_TIMES_OF_DAY = 4;

// Map from int(TimeOfDay) to their names
// "None",
// "Morning",
// "Midday",
// "Evening",
// "Midnight"
extern const std::array<std::string, NUM_TIME_CHANGE_CHOICES> TIME_OF_DAY_NAMES;

// Use one char to represent a time of day
char timeOfDayOneLetter(TimeOfDay time);

// From time of day name (listed in TIME_OF_DAY_NAMES) to enum
TimeOfDay get_time_of_day(const std::string& name);

}
}
}
#endif
