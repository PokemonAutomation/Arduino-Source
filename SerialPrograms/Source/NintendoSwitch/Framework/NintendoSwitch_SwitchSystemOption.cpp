/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
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

#if 0
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
#endif
Color pick_color(
    const ControllerFeatures& required_features,
    FasterIfTickPrecise faster_if_tick_precise
){
    if (required_features.contains(ControllerFeature::NintendoSwitch_DateSkip)){
        return COLOR_RED;
    }
    if (required_features.contains(ControllerFeature::NintendoSwitch_LeftJoycon) ||
        required_features.contains(ControllerFeature::NintendoSwitch_RightJoycon)
    ){
        return COLOR_MAGENTA;
    }
    if (required_features.contains(ControllerFeature::TickPrecise)){
        return COLOR_PURPLE;
    }
    switch (faster_if_tick_precise){
    case FasterIfTickPrecise::MUCH_FASTER:
    case FasterIfTickPrecise::FASTER:
        return COLOR_DARKGREEN;
    case FasterIfTickPrecise::NOT_FASTER:
        return COLOR_BLUE;
    }

    return COLOR_BLUE;
}


const std::string SwitchSystemOption::JSON_CONTROLLER   = "Controller";
const std::string SwitchSystemOption::JSON_CAMERA       = "Camera";
const std::string SwitchSystemOption::JSON_VIDEO        = "Video";
const std::string SwitchSystemOption::JSON_AUDIO        = "Audio";
const std::string SwitchSystemOption::JSON_OVERLAY      = "Overlay";
const std::string SwitchSystemOption::JSON_CONSOLE_TYPE = "ConsoleType";


SwitchSystemOption::SwitchSystemOption(
    const ControllerFeatures& required_features,
    bool allow_commands_while_running
)
    : m_required_features(required_features)
    , m_allow_commands_while_running(allow_commands_while_running)
{}
SwitchSystemOption::SwitchSystemOption(
    const ControllerFeatures& required_features,
    bool allow_commands_while_running,
    const JsonValue& json
)
    : SwitchSystemOption(required_features, allow_commands_while_running)
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

