/*  Ingo Opponent Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoOpponent_H
#define PokemonAutomation_PokemonLA_IngoOpponent_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class IngoOpponents{
    Wenton,
    Bren,
    Zisu,
    Akari_Rei,
    Kamado,
    Beni,
    Ingo,
    Ingo_Tougher,
    Mai,
    Sabi,
    Ress,
    Ingo_Alphas,
    END_LIST,
};
extern const char* INGO_OPPONENT_STRINGS[];

struct IngoOpponentMenuLocation{
    int8_t page;
    int8_t index;
};
extern const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V10[];
extern const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V12[];


class IngoOpponentOption : public EnumDropdownOption<IngoOpponents>{
public:
    IngoOpponentOption();
};



}
}
}
#endif
