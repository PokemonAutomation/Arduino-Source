/*  Berry Selector, UI component to select multiple berries
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BerrySelectOption_H
#define PokemonAutomation_PokemonBDSP_BerrySelectOption_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class BerrySelectCell : public StringSelectCell{
public:
    BerrySelectCell(const std::string& default_slug);
};




}
}
}
#endif
