/*  Pokemon Ball Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokemonBallSelect_H
#define PokemonAutomation_PokemonSwSh_PokemonBallSelect_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class PokemonBallSelectCell : public StringSelectCell{
public:
    PokemonBallSelectCell(const std::string& default_slug = "");
};


class PokemonBallSelectOption : public StringSelectOption{
public:
    PokemonBallSelectOption(
        std::string label,
        LockMode lock_while_running,
        const std::string& default_slug = ""
    );
};


}
}
}
#endif
