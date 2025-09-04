/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
//#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SwitchSystemOption.h"
//#include "UI/NintendoSwitch_SwitchSystemWidget.h"

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


const std::string SwitchSystemOption::JSON_CONTROLLER   = "Controller";
const std::string SwitchSystemOption::JSON_CAMERA       = "Camera";
const std::string SwitchSystemOption::JSON_VIDEO        = "Video";
const std::string SwitchSystemOption::JSON_AUDIO        = "Audio";
const std::string SwitchSystemOption::JSON_OVERLAY      = "Overlay";
const std::string SwitchSystemOption::JSON_CONSOLE_TYPE = "ConsoleType";


SwitchSystemOption::SwitchSystemOption(
    bool allow_commands_while_running
)
    : m_allow_commands_while_running(allow_commands_while_running)
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
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
//    json_get_bool(m_settings_visible, obj, "SettingsVisible");
    const JsonValue* value;
    value = obj->get_value(JSON_CONTROLLER);
    if (value){
        m_controller.load_json(*value);
    }
    value = obj->get_value(JSON_VIDEO);
    if (value){
        m_video.load_json(*value);
    }
    value = obj->get_value(JSON_AUDIO);
    if (value){
        m_audio.load_json(*value);
    }
    value = obj->get_value(JSON_OVERLAY);
    if (value){
        m_overlay.load_json(*value);
    }
    value = obj->get_value(JSON_CONSOLE_TYPE);
    if (value){
        m_console_type.load_json(*value);
    }
}
JsonValue SwitchSystemOption::to_json() const{
    JsonObject root;
//    root.insert("SettingsVisible", m_settings_visible);
    root[JSON_CONTROLLER] = m_controller.to_json();
    root[JSON_VIDEO] = m_video.to_json();
    root[JSON_AUDIO] = m_audio.to_json();
    root[JSON_OVERLAY] = m_overlay.to_json();
    root[JSON_CONSOLE_TYPE] = m_console_type.to_json();

    return root;
}







}
}

