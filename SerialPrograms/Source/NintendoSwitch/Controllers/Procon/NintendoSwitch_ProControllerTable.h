/*  Nintendo Switch Pro Controller Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ProControllerTable_H
#define PokemonAutomation_NintendoSwitch_ProControllerTable_H

#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Controllers/ControllerStateTable.h"
#include "NintendoSwitch_ProControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class ProControllerStateRow : public ControllerStateRow, public ConfigOption::Listener{
public:
    ~ProControllerStateRow();
    ProControllerStateRow(EditableTableOption& parent_table);

    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void get_state(ProControllerState& state) const;
    virtual std::unique_ptr<ControllerState> get_state(Milliseconds& duration) const override;

private:
    virtual void on_config_value_changed(void* object) override;


private:
    MillisecondsCell DURATION;
    StringCell ACTION;
    CheckboxDropdownCell<Button> BUTTONS;
    EnumDropdownCell<DpadPosition> DPAD;
    FloatingPointCell LEFT_JOYSTICK_X;
    FloatingPointCell LEFT_JOYSTICK_Y;
    FloatingPointCell RIGHT_JOYSTICK_X;
    FloatingPointCell RIGHT_JOYSTICK_Y;
};


void register_procon_tables();

std::string get_controller_action(ProControllerState& state);


}
}
#endif
