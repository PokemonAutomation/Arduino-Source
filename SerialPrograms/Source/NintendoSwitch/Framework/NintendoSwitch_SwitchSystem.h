/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystem_H
#define PokemonAutomation_NintendoSwitch_SwitchSystem_H

#include "CommonFramework/ControllerDevices/SerialSelector.h"
#include "CommonFramework/VideoPipeline/CameraSelector.h"
#include "NintendoSwitch_SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSystemWidget;

class SwitchSystemFactory : public SwitchSetupFactory{
    static const QString JSON_SERIAL;
    static const QString JSON_CAMERA;
    static const QString JSON_CAMERA_INDEX;
    static const QString JSON_CAMERA_RESOLUTION;

public:
    SwitchSystemFactory(
        QString label, std::string logger_tag,
        PABotBaseLevel min_pabotbase, FeedbackType feedback
    );
    SwitchSystemFactory(
        QString label, std::string logger_tag,
        PABotBaseLevel min_pabotbase, FeedbackType feedback,
        const QJsonValue& json
    );
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    const QSerialPortInfo* port() const;

    SwitchSetupWidget* make_ui(QWidget& parent, Logger& logger, uint64_t program_id) override;

private:
    friend class SwitchSystemWidget;

    QString m_label;
    std::string m_logger_tag;

    SerialSelector m_serial;
    CameraSelector m_camera;
};





}
}
#endif
