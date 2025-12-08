/*  Nintendo Switch Joycon Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_JoyconTable_H
#define PokemonAutomation_NintendoSwitch_JoyconTable_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.h"
#include "Controllers/ControllerStateTable.h"
#include "NintendoSwitch_JoyconState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class JoyconStateRow : public ControllerStateRow{
public:
    JoyconStateRow(EditableTableOption& parent_table);

    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void get_state(JoyconState& state) const;
    virtual std::unique_ptr<ControllerState> get_state(Milliseconds& duration) const override;

private:
    MillisecondsCell DURATION;
    CheckboxDropdownCell<Button> BUTTONS;
    SimpleIntegerCell<uint8_t> JOYSTICK_X;
    SimpleIntegerCell<uint8_t> JOYSTICK_Y;
};


void register_joycon_tables();



}
}
#endif
