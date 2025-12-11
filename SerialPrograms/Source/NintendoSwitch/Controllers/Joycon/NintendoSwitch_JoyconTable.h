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
#include "Common/Cpp/Options/StringOption.h"
#include "Controllers/ControllerStateTable.h"
#include "NintendoSwitch_JoyconState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class JoyconStateRow : public ControllerStateRow, public ConfigOption::Listener{
public:
    ~JoyconStateRow();
    JoyconStateRow(EditableTableOption& parent_table);

    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void get_state(JoyconState& state) const;
    virtual std::unique_ptr<ControllerState> get_state(Milliseconds& duration) const override;

private:
    virtual void on_config_value_changed(void* object) override;


private:
    MillisecondsCell DURATION;
    CheckboxDropdownCell<Button> BUTTONS;
    SimpleIntegerCell<uint8_t> JOYSTICK_X;
    SimpleIntegerCell<uint8_t> JOYSTICK_Y;
    StringCell ACTION;
};


void register_joycon_tables();

std::string get_controller_action(JoyconState& state);



}
}
#endif
