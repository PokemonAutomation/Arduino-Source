/*  Hyperspace Reward Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceRewardOption_H
#define PokemonAutomation_PokemonLZA_HyperspaceRewardOption_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class HyperspaceRewardCell : public StringSelectCell{
public:
    HyperspaceRewardCell(const std::string& default_slug);
};

}
}
}
#endif
