/*  Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemOption_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemOption_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/ControllerDevices/SerialPortOption.h"
#include "CommonFramework/VideoPipeline/CameraOption.h"
#include "NintendoSwitch_SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSystemWidget;



Color pick_color(FeedbackType feedback, PABotBaseLevel size);


class SwitchSystemOption{
    static const std::string JSON_SERIAL;
    static const std::string JSON_CAMERA;
    static const std::string JSON_AUDIO;

public:
    SwitchSystemOption(
        size_t console_id,
        PABotBaseLevel min_pabotbase,
        bool allow_commands_while_running
    );
    SwitchSystemOption(
        size_t console_id,
        PABotBaseLevel min_pabotbase,
        bool allow_commands_while_running,
        const JsonValue& json
    );
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    SwitchSystemWidget* make_ui(QWidget& parent, Logger& raw_logger, uint64_t program_id);


private:
    friend class SwitchSystemSession;
    friend class SwitchSystemWidget;

    const bool m_allow_commands_while_running;

    size_t m_console_id;
    std::string m_logger_tag;

    SerialPortOption m_serial;
    CameraOption m_camera;
    AudioOption m_audio;
};





}
}
#endif
