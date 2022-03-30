/*  Custom Path Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_CustomPathTableTable_H
#define PokemonAutomation_PokemonLA_CustomPathTableTable_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



enum class PathAction{
    NO_ACTION,
    CHANGE_MOUNT,
    ROTATE_CAMERA,
    MOVE_FORWARD,
    JUMP,
    WAIT,
};

enum class PathMount{
    NO_MOUNT,
    WYRDEER,
    URSALUNA,
    BASCULEGION,
    SNEASLER,
    BRAVIARY,
};

enum class PathSpeed{
    NORMAL_SPEED,
    SLOW_SPEED,
    RUN,
    DASH,
    DASH_B_SPAM,
    DIVE,
};

class ActionParameterWidget;

class CustomPathTableRow : public EditableTableRow{
public:
    CustomPathTableRow();
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    friend class ActionParameterWidget;
    QWidget* make_action_box(QWidget& parent, PathAction& action, ActionParameterWidget* parameterWidget);

public:
    PathAction action = PathAction::NO_ACTION;
    PathMount mount = PathMount::NO_MOUNT;
    int16_t camera_turn_ticks = 0;
    uint16_t move_forward_ticks = 0;
    PathSpeed move_speed = PathSpeed::NORMAL_SPEED;
    uint16_t jump_wait_ticks = 0;
    uint16_t wait_ticks = 0;
};

class CustomPathTableTableFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

// A program option to build a custom path to navigate the map
class CustomPathTableTable : public ConfigOption{
public:
    CustomPathTableTable();

    size_t num_actions() const { return m_table.size(); }

    const CustomPathTableRow& get_action(size_t action_index) const {
        return static_cast<const CustomPathTableRow&>(m_table[action_index]);
    }

    // // Get which style to use according to the info in the table.
    // // pokemon: pokemon index, usually at range [0, 5]
    // // move: move index, range [0, 3]
    // MoveStyle get_style(size_t pokemon, size_t move);

    // // Whether to switch the pokemon at current turns.
    // // pokemon: pokemon index, usually at range [0, 5]
    // // num_turns: num turns passed so far since the pokemon is sent to the battle.
    // bool switch_pokemon(size_t pokemon, size_t num_turns);

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    CustomPathTableTableFactory m_factory;
    EditableTableOption m_table;
};





}
}
}
#endif
