/*  Catchability Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Catchability_H
#define PokemonAutomation_PokemonSwSh_Catchability_H

#include "Common/Cpp/Options/DropdownOption.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class CatchabilitySelectorOption : public DropdownOption{
public:
    CatchabilitySelectorOption()
        : DropdownOption(
            "<b>" + Pokemon::STRING_POKEMON + " Catchability</b>",
            {
                "Always Catchable",
                "Maybe Uncatchable",
                "Never Catchable",
            },
            0
        )
    {}

    operator Catchability() const{ return (Catchability)(size_t)*this; }
};



}
}
}
#endif
