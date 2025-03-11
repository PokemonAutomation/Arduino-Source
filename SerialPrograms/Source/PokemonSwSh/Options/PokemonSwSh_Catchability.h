/*  Catchability Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Catchability_H
#define PokemonAutomation_PokemonSwSh_Catchability_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum class Catchability{
    ALWAYS_CATCHABLE,
    MAYBE_UNCATCHABLE,
    ALWAYS_UNCATCHABLE,
};


class CatchabilitySelectorOption : public EnumDropdownOption<Catchability>{
public:
    CatchabilitySelectorOption()
        : EnumDropdownOption<Catchability>(
            "<b>" + Pokemon::STRING_POKEMON + " Catchability</b>",
            {
                {Catchability::ALWAYS_CATCHABLE,    "always", "Always Catchable"},
                {Catchability::MAYBE_UNCATCHABLE,   "maybe", "Maybe Uncatchable"},
                {Catchability::ALWAYS_UNCATCHABLE,  "never", "Never Catchable"},
            },
            LockMode::LOCK_WHILE_RUNNING,
            Catchability::ALWAYS_CATCHABLE
        )
    {}
};



}
}
}
#endif
