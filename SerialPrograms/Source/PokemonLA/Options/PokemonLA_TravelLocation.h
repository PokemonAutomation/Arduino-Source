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


class TravelLocationOption : public IntegerEnumDropdownOption{
public:
    TravelLocationOption();
    operator TravelLocation() const;
};





}
}
}
#endif
