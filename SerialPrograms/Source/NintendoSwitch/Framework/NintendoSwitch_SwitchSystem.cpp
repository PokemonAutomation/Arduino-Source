/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SwitchSystem.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


const std::string SwitchSystemFactory::JSON_SERIAL  = "Serial";
const std::string SwitchSystemFactory::JSON_CAMERA  = "Camera";
const std::string SwitchSystemFactory::JSON_AUDIO   = "Audio";


SwitchSystemFactory::SwitchSystemFactory(
    size_t console_id,
    PABotBaseLevel min_pabotbase,
    FeedbackType feedback, bool allow_commands_while_running
)
    : SwitchSetupFactory(min_pabotbase, feedback, allow_commands_while_running)
    , m_console_id(console_id)
    , m_logger_tag("Console " + std::to_string(console_id))
//    , m_settings_visible(true)
    , m_serial("<b>Serial Port:</b>", min_pabotbase)
    , m_camera(DEFAULT_RESOLUTION)
    , m_audio()
{}
SwitchSystemFactory::SwitchSystemFactory(
    size_t console_id,
    PABotBaseLevel min_pabotbase,
    FeedbackType feedback, bool allow_commands_while_running,
    const JsonValue& json
)
    : SwitchSystemFactory(
          console_id,
          min_pabotbase,
          feedback, allow_commands_while_running
     )
{
    load_json(json);
}
void SwitchSystemFactory::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
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
}
JsonValue SwitchSystemFactory::to_json() const{
    JsonObject root;
//    root.insert("SettingsVisible", m_settings_visible);
    root[JSON_SERIAL] = m_serial.to_json();
    root[JSON_CAMERA] = m_camera.to_json();
    root[JSON_AUDIO] = m_audio.to_json();
    return root;
}

const QSerialPortInfo* SwitchSystemFactory::port() const{
    return m_serial.port();
}

SwitchSetupWidget* SwitchSystemFactory::make_ui(QWidget& parent, LoggerQt& raw_logger, uint64_t program_id){
    return new SwitchSystemWidget(parent, *this, raw_logger, program_id);
}






}
}

