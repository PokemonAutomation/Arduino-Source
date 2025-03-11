/*  Time of Day Change
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_TimeOfDayChange_H
#define PokemonAutomation_PokemonLA_TimeOfDayChange_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"
#include "PokemonLA/PokemonLA_Locations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// From a camp teleport location, move to the tent and change time of day.
// if `target_time` is TimeOfDay::NONE, it resets for only a while, healing the pokemon but no change of time.
void change_time_of_day_at_tent(
    VideoStream& stream, ProControllerContext& context,
    TimeOfDay target_time,
    Camp camp
);



}
}
}
#endif
