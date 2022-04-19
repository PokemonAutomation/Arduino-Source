/*  Travel Location
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "Common/Cpp/Exceptions.h"
#include "PokemonLA_TravelLocation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TravelLocationOption::TravelLocationOption()
    : EnumDropdownOption(
        "<b>Start Location:</b><br>Travel from this location.",
        TravelLocations::instance().all_location_names(),
        0
    )
{}

TravelLocationOption::operator TravelLocation() const{
    size_t index = (size_t)*this;
    const QString& label = this->case_list()[index];
    std::string str = label.toStdString();
    const TravelLocation* location = TravelLocations::instance().get_from_name(str);
    if (location == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid TravelLocation: " + str);
    }
    return *location;
}





}
}
}
