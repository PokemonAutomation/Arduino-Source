/*  Singles AI Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_SinglesAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



SinglesAIOption::~SinglesAIOption() = default;
SinglesAIOption::SinglesAIOption(bool trainer_battle)
    : GroupOption("Battle AI", LockMode::UNLOCK_WHILE_RUNNING)
    , description(
        "Move Tables: Run these sequence of moves for the corresponding " + STRING_POKEMON + " in your party. "
        "When the end of the table is reached, the last entry will be repeated "
        "until the battle is won or the " + STRING_POKEMON +" faints. "
        "If your " + STRING_POKEMON + " faints, the program will switch to the "
        "next available " + STRING_POKEMON + " and run its table. "
        "Changes to these tables take effect the next the " + STRING_POKEMON + " is sent out."
    )
    , MOVE_TABLES(6)
{
    MOVE_TABLES.emplace_back("<b>1st " + STRING_POKEMON + " Move Table:</b>", trainer_battle);
    MOVE_TABLES.emplace_back("<b>2nd " + STRING_POKEMON + " Move Table:</b>", trainer_battle);
    MOVE_TABLES.emplace_back("<b>3rd " + STRING_POKEMON + " Move Table:</b>", trainer_battle);
    MOVE_TABLES.emplace_back("<b>4th " + STRING_POKEMON + " Move Table:</b>", trainer_battle);
    MOVE_TABLES.emplace_back("<b>5th " + STRING_POKEMON + " Move Table:</b>", trainer_battle);
    MOVE_TABLES.emplace_back("<b>6th " + STRING_POKEMON + " Move Table:</b>", trainer_battle);

    PA_ADD_STATIC(description);
    size_t c = 0;
    for (SinglesMoveTable& table : MOVE_TABLES){
        add_option(table, "MOVE_TABLE" + std::to_string(++c));
    }
}





}
}
}
