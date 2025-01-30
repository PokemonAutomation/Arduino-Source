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

Color pick_color(FeedbackType feedback, PABotBaseLevel size){
    switch (size){
    case PABotBaseLevel::NOT_PABOTBASE:
        return Color();
    case PABotBaseLevel::PABOTBASE_12KB:
        if (feedback == FeedbackType::REQUIRED){
            return COLOR_DARKGREEN;
        }else if (feedback == FeedbackType::VIDEO_AUDIO || feedback == FeedbackType::VIDEO_AUDIO_GBA){
            return COLOR_GREEN2;
        }else{
            return COLOR_BLUE;
        }
    case PABotBaseLevel::PABOTBASE_31KB:
        return (feedback == FeedbackType::REQUIRED || feedback == FeedbackType::VIDEO_AUDIO || feedback == FeedbackType::VIDEO_AUDIO_GBA) ? COLOR_PURPLE : COLOR_RED;
    }
    return Color();
}



const std::string SwitchSystemOption::JSON_SERIAL  = "Serial";
const std::string SwitchSystemOption::JSON_CAMERA  = "Camera";
const std::string SwitchSystemOption::JSON_AUDIO   = "Audio";
const std::string SwitchSystemOption::JSON_OVERLAY = "Overlay";


SwitchSystemOption::SwitchSystemOption(
    PABotBaseLevel min_pabotbase,
    bool allow_commands_while_running
)
    : m_allow_commands_while_running(allow_commands_while_running)
    , m_serial(min_pabotbase)
    , m_camera(DEFAULT_RESOLUTION)
//    , m_audio()
{}
SwitchSystemOption::SwitchSystemOption(
    PABotBaseLevel min_pabotbase,
    bool allow_commands_while_running,
    const JsonValue& json
)
    : SwitchSystemOption(
          min_pabotbase,
          allow_commands_while_running
     )
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
    value = obj->get_value(JSON_SERIAL);
    if (value){
        m_serial.load_json(*value);
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
    root[JSON_SERIAL] = m_serial.to_json();
    root[JSON_CAMERA] = m_camera.to_json();
    root[JSON_AUDIO] = m_audio.to_json();
    root[JSON_OVERLAY] = m_overlay.to_json();
    return root;
}







}
}

