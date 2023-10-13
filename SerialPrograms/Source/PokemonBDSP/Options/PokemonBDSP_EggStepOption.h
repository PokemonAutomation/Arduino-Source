/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggStepOption_H
#define PokemonAutomation_PokemonBDSP_EggStepOption_H

#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{





class EggStepCountOption : public StringSelectOption{
public:
    EggStepCountOption();

    operator uint16_t() const;
};



}
}
}
#endif

