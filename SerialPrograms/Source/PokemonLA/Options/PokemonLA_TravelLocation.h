/*  Travel Location
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_TravelLocation_H
#define PokemonAutomation_PokemonLA_TravelLocation_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


// Warp points in wild areas + custom location after hardcoded movement + ancient retreat
class WildTravelLocationOption : public IntegerEnumDropdownOption{
public:
    WildTravelLocationOption();
    operator TravelLocation() const;
};

// All locations in WildTravelLocationOption + Jubilife village warp points
class AllTravelLocationOption : public IntegerEnumDropdownOption{
public:
    AllTravelLocationOption();
    operator TravelLocation() const;
};



}
}
}
#endif
