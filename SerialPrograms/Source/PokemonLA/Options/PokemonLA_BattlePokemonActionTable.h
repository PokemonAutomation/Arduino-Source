/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Tables to set moves and switching durign battle.
 *
 */

#ifndef PokemonAutomation_PokemonLA_BattlePokemonActionTable_H
#define PokemonAutomation_PokemonLA_BattlePokemonActionTable_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class MoveStyle{
    NoStyle,
    Agile,
    Strong,
};

extern const QString MoveStyle_NAMES[3];

// Used by IngoBattleGrinder, for each pokemon, set what style each move to use
// and when to switch the pokemon.
class BattlePokemonActionRow : public EditableTableRow{
public:
    BattlePokemonActionRow();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

public:
    MoveStyle style[4] = {MoveStyle::NoStyle, MoveStyle::NoStyle, MoveStyle::NoStyle, MoveStyle::NoStyle};
    bool switch_pokemon = false;
    uint16_t num_turns_to_switch = 1;
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
    MoveStyle get_style(size_t pokemon, size_t move);

    // Whether to switch the pokemon at current turns.
    // pokemon: pokemon index, usually at range [0, 5]
    // num_turns: num turns passed so far since the pokemon is sent to the battle.
    bool switch_pokemon(size_t pokemon, size_t num_turns);

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

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
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
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

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    OneMoveBattlePokemonActionTableFactory m_factory;
    EditableTableOption m_table;
};




}
}
}
#endif
