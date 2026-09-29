/*  Battle AI Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BattleAIOption_H
#define PokemonAutomation_PokemonLZA_BattleAIOption_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


enum class BattleAIMode{
    BlindMash,
    TypeEffectiveness,
};


class BattleAIOption : public GroupOption{
public:
    BattleAIOption();

public:
    EnumDropdownOption<BattleAIMode> MODE;
    BooleanCheckBoxOption USE_PLUS_MOVES;
};



}
}
}
#endif
