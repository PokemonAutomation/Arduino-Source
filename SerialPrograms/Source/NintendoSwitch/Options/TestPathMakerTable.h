#ifndef TESTPATHMAKERTABLE_H
#define TESTPATHMAKERTABLE_H

#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

enum class PathAction{
    NO_ACTION,
    LEFT_JOYSTICK,
    RIGHT_JOYSTICK,
    B,
    A,
    Y,
    X,
    R,
    L,
    ZR,
    ZL,
    PLUS,
    MINUS,
    DPADLEFT,
    DPADRIGHT,
    DPADUP,
    DPADDOWN,
    WAIT
};

extern const std::string PathAction_NAMES[];

class ActionParameterWidget;

class TestPathMakerTableRow : public EditableTableRow{
public:
    TestPathMakerTableRow();
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    friend class ActionParameterWidget;
    QWidget* make_action_box(QWidget& parent, PathAction& action, ActionParameterWidget* parameterWidget);

public:
    PathAction action = PathAction::NO_ACTION;

    uint16_t button_hold_ticks = 0;
    uint16_t button_release_ticks = 0;

    uint16_t wait_ticks = 0;

    uint8_t x_axis = 0;
    uint8_t y_axis = 0;
};

class TestPathMakerTableFactory : public EditableTableFactory{
public:
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};

// A program option to build a custom path to navigate the map
class TestPathMakerTable : public BatchOption{
public:
    TestPathMakerTable();

    size_t num_actions() const { return PATH.size(); }
    const TestPathMakerTableRow& get_action(size_t action_index) const {
        return static_cast<const TestPathMakerTableRow&>(PATH[action_index]);
    }

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults() const;

private:
    friend class TestPathMakerTableWidget;
    TestPathMakerTableFactory m_factory;
    EditableTableOption PATH;
};

}
}
#endif
