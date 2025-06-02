/*  Turbo Macro Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboMacroTable_H
#define PokemonAutomation_NintendoSwitch_TurboMacroTable_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


enum class TurboMacroAction{
    NO_ACTION,
    LEFT_JOYSTICK,
    RIGHT_JOYSTICK,
    LEFT_JOY_CLICK,
    RIGHT_JOY_CLICK,
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
const EnumDropdownDatabase<TurboMacroAction>& TurboMacroAction_Database();


class TurboMacroCell : public BatchOption, private ConfigOption::Listener{
public:
    ~TurboMacroCell();
    void operator=(const TurboMacroCell& x);
    TurboMacroCell(EnumDropdownCell<TurboMacroAction>& action);

    virtual void on_config_value_changed(void* object) override;

private:
    EnumDropdownCell<TurboMacroAction>& m_action;

public:
    SimpleIntegerOption<uint8_t> x_axis;
    SimpleIntegerOption<uint8_t> y_axis;

    MillisecondsOption button_hold;
    MillisecondsOption button_release;
    MillisecondsOption wait;
//    SimpleIntegerOption<uint16_t> button_hold_ticks;
//    SimpleIntegerOption<uint16_t> button_release_ticks;
//    SimpleIntegerOption<uint16_t> wait_ticks;
};


class TurboMacroRow : public EditableTableRow{
public:
    TurboMacroRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    EnumDropdownCell<TurboMacroAction> action;
    TurboMacroCell parameters;
};


class TurboMacroTable : public EditableTableOption_t<TurboMacroRow>{
public:
    TurboMacroTable();
    virtual std::vector<std::string> make_header() const override;


private:

};







}
}
#endif
