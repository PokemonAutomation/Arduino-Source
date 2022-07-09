/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Tables to set moves and switching during battle.
 *
 */

#ifndef PokemonAutomation_PokemonLA_BattlePokemonActionTable_H
#define PokemonAutomation_PokemonLA_BattlePokemonActionTable_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/EditableTableOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class MoveStyle{
    NoStyle,
    Agile,
    Strong,
};

extern const std::string MoveStyle_NAMES[3];

// Used by IngoBattleGrinder, for each pokemon, set what style each move to use
// and when to switch the pokemon.
class BattlePokemonActionRow : public EditableTableRow{
public:
    BattlePokemonActionRow();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

public:
    // Move style (agile/strong) of each move
    MoveStyle style[4] = {MoveStyle::NoStyle, MoveStyle::NoStyle, MoveStyle::NoStyle, MoveStyle::NoStyle};
    // Whether to switch pokemon after a number of turns
    bool switch_pokemon = false;
    // The number of turns to switch
    uint16_t num_turns_to_switch = 1;
    // Whether to stop battling after a number of move attempts from a pokemon
    bool stop_after_num_moves = false;
    // The target number of move attempts after which to stop
    uint16_t num_moves_to_stop = 25;
};

class BattlePokemonActionTableFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

// A program option about which pokemon to use which move styles during battle, and whether to switch out
// after some turns.
// This table option is used in IngoBattleGrinder program to set the action of each pokemon sent out to
// battle.
// The pokemon order is defined as the order they are sent onto the battle.
class BattlePokemonActionTable : public ConfigOption{
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

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    BattlePokemonActionTableFactory m_factory;
    EditableTableOption m_table;
};




// Used by MagikarpMoveGrinder, for each pokemon, set what style the first move to use
class OneMoveBattlePokemonActionRow : public EditableTableRow{
public:
    OneMoveBattlePokemonActionRow();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

public:
    MoveStyle style = MoveStyle::NoStyle;
};

class OneMoveBattlePokemonActionTableFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

// A program option used by MagikarpMoveGrinder. To easily grind pokemon moves against a Magikarp which
// only knows Splash.
// Each row in the table option sets which style the first move of a pokemon to use on the Magikarp.
// Since it's most efficient to grind non-damaging moves on Magikarp, the program is designed to grind
// only non-damaging moves. There are not many pokemon whose pokedex researches require grinding more
// than one non-damaging moves, so the program only grinds one move (the first move) per pokemon.
// The pokemon order is defined as the order they are sent onto the battle.
class OneMoveBattlePokemonActionTable : public ConfigOption{
public:
    OneMoveBattlePokemonActionTable();

    size_t num_pokemon() const { return m_table.size(); }

    // Get which style to use according to the info in the table.
    // pokemon: pokemon index, range [0, 5]
    MoveStyle get_style(size_t pokemon);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    OneMoveBattlePokemonActionTableFactory m_factory;
    EditableTableOption m_table;
};



class MoveGrinderActionRow : public EditableTableRow {
public:
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

    size_t pokemon_index;
    size_t move_index;
    MoveStyle style = MoveStyle::NoStyle;
    uint16_t attempts = 0;
};

class MoveGrinderActionTableFactory : public EditableTableFactory {
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

struct Move{
    MoveStyle style;
    uint16_t attempts;
};

class MoveGrinderActionTable : public ConfigOption {
public:
    MoveGrinderActionTable();

    Move get_move(size_t pokemon, size_t move) const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    MoveGrinderActionTableFactory m_factory;
    EditableTableOption m_table;
};


}
}
}
#endif
