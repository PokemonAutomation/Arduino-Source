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
const QString SwitchSystemFactory::JSON_CAMERA_INDEX        = "CameraIndex";
const QString SwitchSystemFactory::JSON_CAMERA_RESOLUTION   = "CameraResolution";


SwitchSystemFactory::SwitchSystemFactory(
    QString label, std::string logger_tag,
    PABotBaseLevel min_pabotbase, FeedbackType feedback
)
    : SwitchSetupFactory(min_pabotbase, feedback)
    , m_label(std::move(label))
    , m_logger_tag(std::move(logger_tag))
    , m_serial("<b>Serial Port:</b>", m_logger_tag, min_pabotbase)
    , m_camera(DEFAULT_RESOLUTION, std::move(label))
{}
SwitchSystemFactory::SwitchSystemFactory(
    QString label, std::string logger_tag,
    PABotBaseLevel min_pabotbase, FeedbackType feedback,
    const QJsonValue& json
)
    : SwitchSystemFactory(
          std::move(label),
          std::move(logger_tag),
          min_pabotbase, feedback
     )
{
    load_json(json);
}
void SwitchSystemFactory::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    m_serial.load_json(json_get_value_nothrow(obj, JSON_SERIAL));
    m_camera.load_json(json_get_value_nothrow(obj, JSON_CAMERA));
}
QJsonValue SwitchSystemFactory::to_json() const{
    QJsonObject root;
    root.insert(JSON_SERIAL, m_serial.to_json());
    root.insert(JSON_CAMERA, m_camera.to_json());
    return root;
}

const QSerialPortInfo* SwitchSystemFactory::port() const{
    return m_serial.port();
}

SwitchSetupWidget* SwitchSystemFactory::make_ui(QWidget& parent, Logger& logger, uint64_t program_id){
    return new SwitchSystemWidget(parent, *this, logger, program_id);
}






}
}

