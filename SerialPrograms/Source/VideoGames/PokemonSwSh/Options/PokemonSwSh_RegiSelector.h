/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RegiSelector_H
#define PokemonAutomation_PokemonSwSh_RegiSelector_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum class RegiGolem{
    Regirock,
    Regice,
    Registeel,
    Regieleki,
    Regidrago,
};

class RegiSelectorOption : public EnumDropdownOption<RegiGolem>{
public:
    RegiSelectorOption()
        : EnumDropdownOption<RegiGolem>(
            "<b>Name of Regi:</b>",
            {
                {RegiGolem::Regirock,   "regirock", "Regirock"},
                {RegiGolem::Regice,     "regice", "Regice"},
                {RegiGolem::Registeel,  "registeel", "Registeel"},
                {RegiGolem::Regieleki,  "regieleki", "Regieleki"},
                {RegiGolem::Regidrago,  "regidrago", "Regidrago"},
            },
            LockMode::LOCK_WHILE_RUNNING,
            RegiGolem::Registeel
        )
    {}
};




}
}
}
#endif
