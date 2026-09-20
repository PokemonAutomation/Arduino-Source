/*  Sandwich Ingredients Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichIngredientsOption_H
#define PokemonAutomation_PokemonSV_SandwichIngredientsOption_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class SandwichIngredientsTableCell : public StringSelectCell{
public:
    SandwichIngredientsTableCell(const std::string& default_slug);
};



}
}
}
#endif
