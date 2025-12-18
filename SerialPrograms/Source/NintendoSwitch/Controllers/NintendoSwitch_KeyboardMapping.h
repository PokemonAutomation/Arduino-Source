/*  Nintendo Keyboard Mapping
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_KeyboardMapping_H
#define PokemonAutomation_NintendoSwitch_KeyboardMapping_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController_from_Keyboard.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon_from_Keyboard.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class KeyboardMappingOption : public BatchOption, private ConfigOption::Listener{
public:
    ~KeyboardMappingOption();
    KeyboardMappingOption();

private:
    virtual void load_json(const JsonValue& json) override;
    virtual void on_config_value_changed(void* object) override;

public:
    StaticTextOption DESCRIPTION;
    BooleanCheckBoxOption ADVANCED_MODE;
    ProControllerFromKeyboardTable PRO_CONTROLLER2;
    JoyconFromKeyboardTable LEFT_JOYCON2;
    JoyconFromKeyboardTable RIGHT_JOYCON2;
};



}
}
#endif
