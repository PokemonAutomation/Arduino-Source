/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystem_H
#define PokemonAutomation_NintendoSwitch_SwitchSystem_H

#include "CommonFramework/AudioPipeline/AudioSelector.h"
#include "CommonFramework/ControllerDevices/SerialPortOption.h"
#include "CommonFramework/VideoPipeline/CameraOption.h"
#include "NintendoSwitch_SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSystemWidget;

class SwitchSystemFactory : public SwitchSetupFactory{
    static const std::string JSON_SERIAL;
    static const std::string JSON_CAMERA;
    static const std::string JSON_AUDIO;

public:
    SwitchSystemFactory(
        size_t console_id,
        PABotBaseLevel min_pabotbase,
        FeedbackType feedback, bool allow_commands_while_running
    );
    SwitchSystemFactory(
        size_t console_id,
        PABotBaseLevel min_pabotbase,
        FeedbackType feedback, bool allow_commands_while_running,
        const JsonValue& json
    );
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    const QSerialPortInfo* port() const;

    SwitchSetupWidget* make_ui(QWidget& parent, LoggerQt& raw_logger, uint64_t program_id) override;

private:
    friend class SwitchSystemWidget;

    size_t m_console_id;
    std::string m_logger_tag;

//    bool m_settings_visible;
    SerialPortOption m_serial;
    CameraOption m_camera;
    AudioSelector m_audio;
};





}
}
#endif
