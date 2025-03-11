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


TravelLocationOption::TravelLocationOption()
    : IntegerEnumDropdownOption(
        "<b>Start Location:</b><br>Travel from this location.",
        TravelLocations::instance().database(),
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{}

TravelLocationOption::operator TravelLocation() const{
    size_t index = this->current_value();
    return TravelLocations::instance()[index];
}





}
}
}
