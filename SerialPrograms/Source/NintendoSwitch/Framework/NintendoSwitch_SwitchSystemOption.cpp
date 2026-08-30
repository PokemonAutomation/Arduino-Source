/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

// constexpr Color COLOR_GREEN2(0xff00aa00);

Color pick_color(ProgramControllerClass color_class){
    switch (color_class){
    case ProgramControllerClass::StandardController_NoRestrictions:
        return COLOR_BLUE;
    case ProgramControllerClass::StandardController_PerformanceClassSensitive:
        return COLOR_DARKGREEN;
    case ProgramControllerClass::StandardController_RequiresPrecision:
        return COLOR_PURPLE;
    case ProgramControllerClass::StandardController_WithRestrictions:
        return COLOR_RED;
    case ProgramControllerClass::SpecializedController:
        return COLOR_MAGENTA;
    default:
        return Color();
    }
}


const std::string SwitchSystemOption::JSON_CONSOLE_TYPE = "ConsoleType";


SwitchSystemOption::SwitchSystemOption(
    bool allow_commands_while_running
)
    : ConsoleSystemOption(1, allow_commands_while_running)  //  REMOVE
{}
SwitchSystemOption::SwitchSystemOption(
    bool allow_commands_while_running,
    const JsonValue& json
)
    : SwitchSystemOption(allow_commands_while_running)
{
    load_json(json);
}
void SwitchSystemOption::load_json(const JsonValue& json){
    ConsoleSystemOption::load_json(json);

    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    const JsonValue* value;
    value = obj->get_value(JSON_CONSOLE_TYPE);
    if (value){
        m_console_type.load_json(*value);
    }
}
JsonValue SwitchSystemOption::to_json() const{
    JsonValue ret = ConsoleSystemOption::to_json();
    JsonObject& root = ret.to_object_throw();
    root[JSON_CONSOLE_TYPE] = m_console_type.to_json();
    return ret;
}







}
}

