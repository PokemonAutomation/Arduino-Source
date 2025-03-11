/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggStepOption_H
#define PokemonAutomation_PokemonSwSh_EggStepOption_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




class EggStepCountOption : public StringSelectOption{
public:
    EggStepCountOption();

    operator uint16_t() const;
};




}
}
}
#endif

