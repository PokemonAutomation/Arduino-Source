/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_BattlePokemonActionTable_H
#define PokemonAutomation_PokemonLA_BattlePokemonActionTable_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
// using namespace Pokemon;


enum class MoveStyle{
    NoStyle,
    Agile,
    Strong,
};

extern const QString MoveStyle_NAMES[3];


class BattlePokemonActionRow : public EditableTableRow{
public:
    BattlePokemonActionRow();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_style_box(QWidget& parent, MoveStyle& style);
    QWidget* make_switch_box(QWidget& parent);
    QWidget* make_turns_box(QWidget& parent);

public:
    MoveStyle style[4] = {MoveStyle::NoStyle, MoveStyle::NoStyle, MoveStyle::NoStyle, MoveStyle::NoStyle};
    bool switch_pokemon = false;
    int num_turns_to_switch = 1;
};

class BattlePokemonActionTableFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

// A program option on which pokemon to use which move styles during battle, and whether to switch out
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





}
}
}
#endif
