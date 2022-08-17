/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Tables to set moves and switching during battle.
 *
 */

#ifndef PokemonAutomation_PokemonLA_BattlePokemonActionTable_H
#define PokemonAutomation_PokemonLA_BattlePokemonActionTable_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class MoveStyle{
    NoStyle,
    Agile,
    Strong,
};
extern const std::string MoveStyle_NAMES[3];



class MoveStyleCell : public EnumDropdownCell{
public:
    MoveStyleCell()
        : EnumDropdownCell({"No Style", "Agile", "Strong", }, 0)
    {}

    operator MoveStyle() const{
        return (MoveStyle)(size_t)*this;
    }
    void set(MoveStyle style){
        EnumDropdownCell::set((size_t)style);
    }
};




// A program option about which pokemon to use which move styles during battle, and whether to switch out
// after some turns.
// This table option is used in IngoBattleGrinder program to set the action of each pokemon sent out to
// battle.
// The pokemon order is defined as the order they are sent onto the battle.

class BattlePokemonActionRow : public EditableTableRow2{
public:
    BattlePokemonActionRow();
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

public:
    MoveStyleCell style[4];
    BooleanCheckBoxCell switch_pokemon;
    SimpleIntegerCell<uint16_t> num_turns_to_switch;
    BooleanCheckBoxCell stop_after_num_moves;
    SimpleIntegerCell<uint16_t> num_moves_to_stop;
};

class BattlePokemonActionTable : public EditableTableOption2<BattlePokemonActionRow>{
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
    static std::vector<std::unique_ptr<EditableTableRow2>> make_defaults();
};





// A program option used by MagikarpMoveGrinder. To easily grind pokemon moves against a Magikarp which
// only knows Splash.
// Each row in the table option sets which style the first move of a pokemon to use on the Magikarp.
// Since it's most efficient to grind non-damaging moves on Magikarp, the program is designed to grind
// only non-damaging moves. There are not many pokemon whose pokedex researches require grinding more
// than one non-damaging moves, so the program only grinds one move (the first move) per pokemon.
// The pokemon order is defined as the order they are sent onto the battle.

// Used by MagikarpMoveGrinder, for each pokemon, set what style the first move to use
class OneMoveBattlePokemonActionRow : public EditableTableRow2{
public:
    OneMoveBattlePokemonActionRow();
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

public:
    MoveStyleCell style;
};

class OneMoveBattlePokemonActionTable : public EditableTableOption2<OneMoveBattlePokemonActionRow>{
public:
    OneMoveBattlePokemonActionTable();

    size_t num_pokemon() const{ return current_rows(); }

    // Get which style to use according to the info in the table.
    // pokemon: pokemon index, range [0, 5]
    MoveStyle get_style(size_t pokemon);

private:
    virtual std::vector<std::string> make_header() const;
    std::vector<std::unique_ptr<EditableTableRow2>> make_defaults() const;
};






class MoveGrinderActionRow2 : public EditableTableRow2{
public:
    MoveGrinderActionRow2();
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

public:
    EnumDropdownCell pokemon_index;
    EnumDropdownCell move_index;
    MoveStyleCell style;
    SimpleIntegerCell<uint16_t> attempts;
};

struct Move{
    MoveStyle style;
    uint16_t attempts;
};

class MoveGrinderActionTable : public EditableTableOption2<MoveGrinderActionRow2>{
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
