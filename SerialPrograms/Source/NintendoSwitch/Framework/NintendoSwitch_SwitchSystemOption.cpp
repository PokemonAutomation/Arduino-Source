/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SwitchSystemOption.h"
//#include "UI/NintendoSwitch_SwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

    // constexpr Color COLOR_GREEN2(0xff00aa00);

Color pick_color(FeedbackType feedback){
    switch (feedback){
    case FeedbackType::NONE:
        return COLOR_BLUE;
    case FeedbackType::OPTIONAL_:
        return COLOR_PURPLE;
    case FeedbackType::REQUIRED:
        return COLOR_DARKGREEN;
    case FeedbackType::VIDEO_AUDIO:
        return COLOR_RED;
    }
    return Color();
}



const std::string SwitchSystemOption::JSON_CONTROLLER  = "Controller";
const std::string SwitchSystemOption::JSON_CAMERA  = "Camera";
const std::string SwitchSystemOption::JSON_AUDIO   = "Audio";
const std::string SwitchSystemOption::JSON_OVERLAY = "Overlay";


SwitchSystemOption::SwitchSystemOption(
    const ControllerRequirements& requirements,
    bool allow_commands_while_running
)
    : m_requirements(requirements)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_camera(DEFAULT_RESOLUTION)
//    , m_audio()
{}
SwitchSystemOption::SwitchSystemOption(
    const ControllerRequirements& requirements,
    bool allow_commands_while_running,
    const JsonValue& json
)
    : SwitchSystemOption(requirements, allow_commands_while_running)
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
    value = obj->get_value(JSON_CAMERA);
    if (value){
        m_camera.load_json(*value);
    }
    value = obj->get_value(JSON_AUDIO);
    if (value){
        m_audio.load_json(*value);
    }
    value = obj->get_value(JSON_OVERLAY);
    if (value){
        m_overlay.load_json(*value);
    }
}
JsonValue SwitchSystemOption::to_json() const{
    JsonObject root;
//    root.insert("SettingsVisible", m_settings_visible);
    root[JSON_CONTROLLER] = m_controller.to_json();
    root[JSON_CAMERA] = m_camera.to_json();
    root[JSON_AUDIO] = m_audio.to_json();
    root[JSON_OVERLAY] = m_overlay.to_json();
    return root;
}







}
}

