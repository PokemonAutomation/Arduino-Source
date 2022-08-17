/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokemonNameSelect_H
#define PokemonAutomation_PokemonSwSh_PokemonNameSelect_H

#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class PokemonNameSelectCell : public StringSelectCell{
public:
    PokemonNameSelectCell(const std::string& default_slug = "");
};


class PokemonNameSelectOption : public StringSelectOption{
public:
    PokemonNameSelectOption(std::string label, const std::string& default_slug = "");
};


}
}
}
#endif
