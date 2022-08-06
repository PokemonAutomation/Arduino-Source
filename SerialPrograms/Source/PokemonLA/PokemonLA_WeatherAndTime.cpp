/*  Weather and Time of Day Definitions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "PokemonLA_WeatherAndTime.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



const char* TIME_OF_DAY_NAMES[] = {
    "None",
    "MORNING",
    "MIDDAY",
    "EVENING",
    "MIDNIGHT"
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
}


}
}
}
