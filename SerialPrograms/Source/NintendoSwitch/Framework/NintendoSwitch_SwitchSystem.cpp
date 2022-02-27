/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SwitchSystem.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


const QString SwitchSystemFactory::JSON_SERIAL  = "Serial";
const QString SwitchSystemFactory::JSON_CAMERA  = "Camera";
const QString SwitchSystemFactory::JSON_AUDIO  = "Audio";


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
    const QJsonValue& json
)
    : SwitchSystemFactory(
          console_id,
          min_pabotbase,
          feedback, allow_commands_while_running
     )
{
    load_json(json);
}
void SwitchSystemFactory::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
//    json_get_bool(m_settings_visible, obj, "SettingsVisible");
    m_serial.load_json(json_get_value_nothrow(obj, JSON_SERIAL));
    m_camera.load_json(json_get_value_nothrow(obj, JSON_CAMERA));
    m_audio.load_json(json_get_value_nothrow(obj, JSON_AUDIO));
}
QJsonValue SwitchSystemFactory::to_json() const{
    QJsonObject root;
//    root.insert("SettingsVisible", m_settings_visible);
    root.insert(JSON_SERIAL, m_serial.to_json());
    root.insert(JSON_CAMERA, m_camera.to_json());
    root.insert(JSON_AUDIO, m_audio.to_json());
    return root;
}

const QSerialPortInfo* SwitchSystemFactory::port() const{
    return m_serial.port();
}

SwitchSetupWidget* SwitchSystemFactory::make_ui(QWidget& parent, Logger& raw_logger, uint64_t program_id){
    return new SwitchSystemWidget(parent, *this, raw_logger, program_id);
}






}
}

