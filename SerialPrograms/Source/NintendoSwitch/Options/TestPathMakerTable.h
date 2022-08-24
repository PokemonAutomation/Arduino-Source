/*  Test Path Maker Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PathMakerTable_H
#define PokemonAutomation_NintendoSwitch_PathMakerTable_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"

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
const EnumDatabase<PathAction>& PathAction_Database();


class PathMakerCell : public BatchOption, private ConfigOption::Listener{
public:
    ~PathMakerCell();
    void operator=(const PathMakerCell& x);
    PathMakerCell(EnumDropdownCell<PathAction>& action);

    virtual void value_changed() override;

private:
    EnumDropdownCell<PathAction>& m_action;

public:
    SimpleIntegerOption<uint8_t> x_axis;
    SimpleIntegerOption<uint8_t> y_axis;

    SimpleIntegerOption<uint16_t> button_hold_ticks;
    SimpleIntegerOption<uint16_t> button_release_ticks;
    SimpleIntegerOption<uint16_t> wait_ticks;
};


class PathMakerRow2 : public EditableTableRow2{
public:
    PathMakerRow2();
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    EnumDropdownCell<PathAction> action;
    PathMakerCell parameters;
};


class PathMakerTable : public EditableTableOption_t<PathMakerRow2>{
public:
    PathMakerTable();
    virtual std::vector<std::string> make_header() const override;


private:

};







}
}
#endif
