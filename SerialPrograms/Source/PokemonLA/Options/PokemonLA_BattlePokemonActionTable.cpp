/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_BattlePokemonActionTable.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


const std::string MoveStyle_NAMES[] = {
    "No Style",
    "Agile",
    "Strong",
};
const std::map<std::string, MoveStyle> MoveStyle_MAP{
    {MoveStyle_NAMES[0], MoveStyle::NoStyle},
    {MoveStyle_NAMES[1], MoveStyle::Agile},
    {MoveStyle_NAMES[2], MoveStyle::Strong},
};




BattlePokemonActionRow::BattlePokemonActionRow()
    : switch_pokemon(false)
    , num_turns_to_switch(1)
    , stop_after_num_moves(false)
    , num_moves_to_stop(25, 0)
{
    PA_ADD_OPTION(style[0]);
    PA_ADD_OPTION(style[1]);
    PA_ADD_OPTION(style[2]);
    PA_ADD_OPTION(style[3]);
    PA_ADD_OPTION(switch_pokemon);
    PA_ADD_OPTION(num_turns_to_switch);
    PA_ADD_OPTION(stop_after_num_moves);
    PA_ADD_OPTION(num_moves_to_stop);
}
std::unique_ptr<EditableTableRow2> BattlePokemonActionRow::clone() const{
    std::unique_ptr<BattlePokemonActionRow> ret(new BattlePokemonActionRow());
    ret->style[0].set(style[0]);
    ret->style[1].set(style[1]);
    ret->style[2].set(style[2]);
    ret->style[3].set(style[3]);
    ret->switch_pokemon = (bool)switch_pokemon;
    ret->num_turns_to_switch.set(num_turns_to_switch);
    ret->stop_after_num_moves = (bool)stop_after_num_moves;
    ret->num_moves_to_stop.set(num_moves_to_stop);
    return ret;
}


std::vector<std::unique_ptr<EditableTableRow2>> BattlePokemonActionTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow2>> ret;
    ret.emplace_back(std::make_unique<BattlePokemonActionRow>());
    return ret;
}
BattlePokemonActionTable::BattlePokemonActionTable()
    : EditableTableOption2<BattlePokemonActionRow>(
        "<b>" + STRING_POKEMON + " Action Table:</b><br>"
        "Set what move styles to use and whether to switch the " + STRING_POKEMON + " after some turns.<br><br>"
        "Each row is the action for one " + STRING_POKEMON + ". "
        "The table follows the order that " + STRING_POKEMON + " are sent to battle.<br>"
        "You can also set a target number of move attempts. After it is reached the program will finish the battle and stop.<br><br>"
        "Note: if your second last " + STRING_POKEMON + " faints, the game will send your last " + STRING_POKEMON + " automatically for you.<br>"
        "The program cannot detect this switch as there is no switch selection screen. "
        "Therefore the program will treat it as the same " + STRING_POKEMON + ".",
        make_defaults()
    )
{}
std::vector<std::string> BattlePokemonActionTable::make_header() const{
    return std::vector<std::string>{
        "Move 1 Style",
        "Move 2 Style",
        "Move 3 Style",
        "Move 4 Style",
        "Switch " + STRING_POKEMON,
        "Num Turns to Switch",
        "Limit Move Attempts",
        "Max Move Attempts",
    };
}
MoveStyle BattlePokemonActionTable::get_style(size_t pokemon, size_t move) const{
    std::vector<std::unique_ptr<BattlePokemonActionRow>> table = copy_snapshot();
    if (pokemon >= table.size()){
        return MoveStyle::NoStyle;
    }

    const BattlePokemonActionRow& action = *table[pokemon];
    return action.style[move];
}
bool BattlePokemonActionTable::switch_pokemon(size_t pokemon, size_t num_turns) const{
    std::vector<std::unique_ptr<BattlePokemonActionRow>> table = copy_snapshot();
    if (pokemon >= table.size()){
        return false;
    }

    const BattlePokemonActionRow& action = *table[pokemon];
    return action.switch_pokemon && num_turns >= (size_t)action.num_turns_to_switch;
}
bool BattlePokemonActionTable::stop_battle(size_t pokemon, size_t num_move_attempts) const{
    std::vector<std::unique_ptr<BattlePokemonActionRow>> table = copy_snapshot();
    if (pokemon >= table.size()){
        return false;
    }

    const BattlePokemonActionRow& action = *table[pokemon];
    return action.stop_after_num_moves && num_move_attempts >= (size_t)action.num_moves_to_stop;
}











OneMoveBattlePokemonActionRow::OneMoveBattlePokemonActionRow(){
    PA_ADD_OPTION(style);
}
std::unique_ptr<EditableTableRow2> OneMoveBattlePokemonActionRow::clone() const{
    std::unique_ptr<OneMoveBattlePokemonActionRow> ret(new OneMoveBattlePokemonActionRow());
    ret->style.set(style);
    return ret;
}

std::vector<std::unique_ptr<EditableTableRow2>> OneMoveBattlePokemonActionTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow2>> ret;
    ret.emplace_back(std::make_unique<OneMoveBattlePokemonActionRow>());
    return ret;
}
OneMoveBattlePokemonActionTable::OneMoveBattlePokemonActionTable()
    : EditableTableOption2<OneMoveBattlePokemonActionRow>(
        "<b>" + STRING_POKEMON + " Action Table:</b><br>"
        "Set what move style to use for each " + STRING_POKEMON + " to grind against a Magikarp. "
        "Each row is the action for one " + STRING_POKEMON + ". "
        "The table follows the order that " + STRING_POKEMON + " are sent to battle.",
        make_defaults()
    )
{}
std::vector<std::string> OneMoveBattlePokemonActionTable::make_header() const{
    return std::vector<std::string>{
        "Move Style",
    };
}
MoveStyle OneMoveBattlePokemonActionTable::get_style(size_t pokemon){
    std::vector<std::unique_ptr<OneMoveBattlePokemonActionRow>> table = copy_snapshot();
    if (pokemon >= table.size()){
        return MoveStyle::NoStyle;
    }
    const OneMoveBattlePokemonActionRow& action = *table[pokemon];
    return action.style;
}









MoveGrinderActionRow2::MoveGrinderActionRow2()
    : pokemon_index({
        "1st " + STRING_POKEMON,
        "2nd " + STRING_POKEMON,
        "3rd " + STRING_POKEMON,
        "4th " + STRING_POKEMON,
    }, 0)
    , move_index({
        "1st Move",
        "2nd Move",
        "3rd Move",
        "4th Move",
    }, 0)
    , attempts(1, 1)
{
    PA_ADD_OPTION(pokemon_index);
    PA_ADD_OPTION(move_index);
    PA_ADD_OPTION(style);
    PA_ADD_OPTION(attempts);
}
std::unique_ptr<EditableTableRow2> MoveGrinderActionRow2::clone() const{
    std::unique_ptr<MoveGrinderActionRow2> ret(new MoveGrinderActionRow2());
    ret->pokemon_index.set(pokemon_index);
    ret->move_index.set(move_index);
    ret->style.set(style);
    ret->attempts.set(attempts);
    return ret;
}

MoveGrinderActionTable::MoveGrinderActionTable()
    : EditableTableOption2<MoveGrinderActionRow2>(
        "For every move you want to perform, input the style and the number of attemps you want to achieve."
    )
{}
Move MoveGrinderActionTable::get_move(size_t pokemon, size_t move) const{
    // Pokemon index 4 is gonna be Arceus with powerful moves, just use them with normal style and hope you'll win the battle
    if (pokemon == 4){
        return {MoveStyle::NoStyle, std::numeric_limits<decltype(Move::attempts)>::max()};

    }
    std::vector<std::unique_ptr<MoveGrinderActionRow2>> table = copy_snapshot();
    for (size_t i = 0; i < table.size(); ++i){
        const MoveGrinderActionRow2& action = *table[i];
        if (action.pokemon_index != pokemon){
            continue;
        }
        if (action.move_index != move){
            continue;
        }
        return {action.style , action.attempts};
    }
    return {MoveStyle::NoStyle, 0};
}
std::vector<std::string> MoveGrinderActionTable::make_header() const{
    return std::vector<std::string>{
        "Pokemon index",
        "Move index",
        "Move Style",
        "Move Attempts",
    };
}




}
}
}
