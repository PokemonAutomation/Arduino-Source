/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Tables to set moves and switching during battle.
 *
 */

#ifndef PokemonAutomation_PokemonLA_BattlePokemonActionTable_H
#define PokemonAutomation_PokemonLA_BattlePokemonActionTable_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class MoveStyle{
    NoStyle,
    Agile,
    Strong,
};
const EnumDropdownDatabase<MoveStyle>& MoveStyle_Database();



class MoveStyleCell : public EnumDropdownCell<MoveStyle>{
public:
    MoveStyleCell()
        : EnumDropdownCell<MoveStyle>(
            MoveStyle_Database(),
            LockMode::LOCK_WHILE_RUNNING,
            MoveStyle::NoStyle
        )
    {}
};




// A program option about which pokemon to use which move styles during battle, and whether to switch out
// after some turns.
// This table option is used in IngoBattleGrinder program to set the action of each pokemon sent out to
// battle.
// The pokemon order is defined as the order they are sent onto the battle.

class BattlePokemonActionRow : public EditableTableRow{
public:
    BattlePokemonActionRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    MoveStyleCell style[4];
    BooleanCheckBoxCell switch_pokemon;
    SimpleIntegerCell<uint16_t> num_turns_to_switch;
    BooleanCheckBoxCell stop_after_num_moves;
    SimpleIntegerCell<uint16_t> num_moves_to_stop;
};

class BattlePokemonActionTable : public EditableTableOption_t<BattlePokemonActionRow>{
public:
    BattlePokemonActionTable();

    // Get which style to use according to the info in the table.
    // pokemon: pokemon index, usually at range [0, 5]
    // move: move index, range [0, 3]
    MoveStyle get_style(size_t pokemon, size_t move) const;

    // Whether to switch the pokemon at current turns.
    // pokemon: pokemon index, usually at range [0, 5]
    // num_turns: num turns passed so far since the pokemon is sent to the battle.
    bool switch_pokemon(size_t pokemon, size_t num_turns) const;
    // Whether to stop battling after a certain number of move attempts are made on
    // a pokemon.
    // pokemon: pokemon index, usually at range [0, 5]
    // num_move_attempts: number of move attempts made from this pokemon so far.
    bool stop_battle(size_t pokemon, size_t num_move_attempts) const;

private:
    virtual std::vector<std::string> make_header() const;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





// A program option used by MagikarpMoveGrinder. To easily grind pokemon moves against a Magikarp which
// only knows Splash.
// Each row in the table option sets which style the first move of a pokemon to use on the Magikarp.
// Since it's most efficient to grind non-damaging moves on Magikarp, the program is designed to grind
// only non-damaging moves. There are not many pokemon whose pokedex researches require grinding more
// than one non-damaging moves, so the program only grinds one move (the first move) per pokemon.
// The pokemon order is defined as the order they are sent onto the battle.

// Used by MagikarpMoveGrinder, for each pokemon, set what style the first move to use
class OneMoveBattlePokemonActionRow : public EditableTableRow{
public:
    OneMoveBattlePokemonActionRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    MoveStyleCell style;
};

class OneMoveBattlePokemonActionTable : public EditableTableOption_t<OneMoveBattlePokemonActionRow>{
public:
    OneMoveBattlePokemonActionTable();

    size_t num_pokemon() const{ return current_rows(); }

    // Get which style to use according to the info in the table.
    // pokemon: pokemon index, range [0, 5]
    MoveStyle get_style(size_t pokemon);

private:
    virtual std::vector<std::string> make_header() const;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};






class MoveGrinderActionRow : public EditableTableRow{
public:
    MoveGrinderActionRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    IntegerEnumDropdownCell pokemon_index;
    IntegerEnumDropdownCell move_index;
    MoveStyleCell style;
    SimpleIntegerCell<uint16_t> attempts;
};

struct Move{
    MoveStyle style;
    uint16_t attempts;
};

class MoveGrinderActionTable : public EditableTableOption_t<MoveGrinderActionRow>{
public:
    MoveGrinderActionTable();

    Move get_move(size_t pokemon, size_t move) const;

private:
    virtual std::vector<std::string> make_header() const;
};







}
}
}
#endif
