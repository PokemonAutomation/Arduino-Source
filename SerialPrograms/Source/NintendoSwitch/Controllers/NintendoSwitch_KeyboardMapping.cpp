/*  Nintendo Keyboard Mapping
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch_KeyboardMapping.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




KeyboardMappingOption::~KeyboardMappingOption(){
    ADVANCED_MODE.remove_listener(*this);
}
KeyboardMappingOption::KeyboardMappingOption()
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , DESCRIPTION(
        "The following table is the mapping of keyboard keys to Switch controller presses. "
        "If you wish to remap a key, click on the cell in the \"Key\" column and press the desired key. "
        "You do not need to edit any of the other columns.<br><br>"
        "<font color=\"orange\">Note for keys that change behavior when combined with "
        "SHIFT or CTRL, you should include all of those combinations as well. "
        "For example, the default mapping for the Y button is both '/' and '?' "
        "because they are treated as different keys depending on whether SHIFT "
        "is held down. Letters are exempt from this as both lower and upper case "
        "letters are considered the same.</font>"
        "<br><br>"
        "Advanced users are free to edit the rest of the table. You can create "
        "new mappings or mappings that result in multiple buttons. "
        "For example, there is a special mapping for pressing A + R "
        "simultaneously that is useful for CFW users who are remotely "
        "controlling the program over Team Viewer."
    )
    , ADVANCED_MODE(
        "Unlock entire table (Advanced Mode):",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , LEFT_JOYCON2(true)
    , RIGHT_JOYCON2(false)
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(ADVANCED_MODE);
    PA_ADD_OPTION(PRO_CONTROLLER2);
    PA_ADD_OPTION(LEFT_JOYCON2);
    PA_ADD_OPTION(RIGHT_JOYCON2);
    ADVANCED_MODE.add_listener(*this);
}


void KeyboardMappingOption::load_json(const JsonValue& json){
    BatchOption::load_json(json);
    KeyboardMappingOption::on_config_value_changed(this);
}
void KeyboardMappingOption::on_config_value_changed(void* object){
    PRO_CONTROLLER2.set_advanced_mode(ADVANCED_MODE);
    LEFT_JOYCON2.set_advanced_mode(ADVANCED_MODE);
    RIGHT_JOYCON2.set_advanced_mode(ADVANCED_MODE);
}












}
}
