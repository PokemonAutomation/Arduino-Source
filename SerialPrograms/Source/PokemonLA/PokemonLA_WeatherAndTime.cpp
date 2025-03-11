/*  Weather and Time of Day Definitions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <string>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "PokemonLA_WeatherAndTime.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const std::array<std::string, NUM_WEATHER> WEATHER_NAMES = {
    "Sunny",
    "Cloudy",
    "Rain",
    "Snowy",
    "Drought",
    "Fog",
    "Rainstorm",
    "Snowstorm"
};

Weather get_weather(const std::string& name){
    const auto it = std::find(WEATHER_NAMES.begin(), WEATHER_NAMES.end(), name);
    if (it == WEATHER_NAMES.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown weather name: " + name);
    }

    return Weather(std::distance(WEATHER_NAMES.begin(), it));
}



const std::array<std::string, NUM_TIME_CHANGE_CHOICES> TIME_OF_DAY_NAMES = {
    "None",
    "Morning",
    "Midday",
    "Evening",
    "Midnight"
};

char timeOfDayOneLetter(TimeOfDay time){
    switch (time){
    case TimeOfDay::NONE:
        return 'S'; // "S"ame time, no change on time of day
    case TimeOfDay::MORNING:
        return 'M';
    case TimeOfDay::MIDDAY:
        return 'D';
    case TimeOfDay::EVENING:
        return 'E';
    case TimeOfDay::MIDNIGHT:
        return 'N';
    }
    return '\0';
}

TimeOfDay get_time_of_day(const std::string& name){
    const auto it = std::find(TIME_OF_DAY_NAMES.begin(), TIME_OF_DAY_NAMES.end(), name);
    if (it == TIME_OF_DAY_NAMES.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown time of day name: " + name);
    }

    return TimeOfDay(std::distance(TIME_OF_DAY_NAMES.begin(), it));
}


}
}
}
