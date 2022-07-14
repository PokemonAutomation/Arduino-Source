/*  Custom Path Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_CustomPathTableTable_H
#define PokemonAutomation_PokemonLA_CustomPathTableTable_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "CommonFramework/Options/EditableTableOption.h"
#include "PokemonLA_TravelLocation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



enum class PathAction{
    NO_ACTION,
    CHANGE_MOUNT,
//    ROTATE_CAMERA,
    MOVE_FORWARD,
    MOVE_IN_DIRECTION,
    CENTER_CAMERA,
    JUMP,
    WAIT,
    START_LISTEN,
    END_LISTEN,
};

extern const std::string PathAction_NAMES[];

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
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    friend class ActionParameterWidget;
    QWidget* make_action_box(QWidget& parent, PathAction& action, ActionParameterWidget* parameterWidget);

public:
    PathAction action = PathAction::NO_ACTION;
    PathMount mount = PathMount::NO_MOUNT;

    int16_t camera_turn_ticks = 0;

    PathSpeed move_speed = PathSpeed::NORMAL_SPEED;
    uint16_t move_forward_ticks = 0;
    double left_x = 0;
    double left_y = 0;

    uint16_t jump_wait_ticks = 0;
    uint16_t wait_ticks = 0;
};

class CustomPathTableTableFactory : public EditableTableFactory{
public:
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

// A program option to build a custom path to navigate the map
class CustomPathTable : public BatchOption{
public:
    CustomPathTable();

    const TravelLocationOption& travel_location() const{ return TRAVEL_LOCATION; }

    size_t num_actions() const { return PATH.size(); }
    const CustomPathTableRow& get_action(size_t action_index) const {
        return static_cast<const CustomPathTableRow&>(PATH[action_index]);
    }

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    friend class CustomPathTableWidget;

    TravelLocationOption TRAVEL_LOCATION;
    CustomPathTableTableFactory m_factory;
    EditableTableOption PATH;
};





}
}
}
#endif
