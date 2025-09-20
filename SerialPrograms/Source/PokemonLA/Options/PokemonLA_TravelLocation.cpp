/*  Travel Location
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "PokemonLA_TravelLocation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


WildTravelLocationOption::WildTravelLocationOption()
    : IntegerEnumDropdownOption(
        "<b>Start Location:</b><br>Travel from this location.",
        TravelLocations::instance().database_outside_village(),
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{}

WildTravelLocationOption::operator TravelLocation() const{
    size_t index = this->current_value();
    return TravelLocations::instance()[index];
}


AllTravelLocationOption::AllTravelLocationOption()
    : IntegerEnumDropdownOption(
        "<b>Start Location:</b><br>Travel from this location.",
        TravelLocations::instance().database_including_village(),
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{}

AllTravelLocationOption::operator TravelLocation() const{
    size_t index = this->current_value();
    return TravelLocations::instance()[index];
}





}
}
}
