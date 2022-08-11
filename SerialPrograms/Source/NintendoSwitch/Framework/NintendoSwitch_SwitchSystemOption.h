/*  Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This class represents the serializable state of a Switch console.
 *  Specifially, holds the settings of:
 *      -   Serial Port
 *      -   Camera
 *      -   Audio
 *
 *  This class maintains no runtime state or UI and is not thread-safe.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemOption_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemOption_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/ControllerDevices/SerialPortOption.h"
#include "CommonFramework/VideoPipeline/CameraOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


Color pick_color(FeedbackType feedback, PABotBaseLevel size);


class SwitchSystemOption{
    static const std::string JSON_SERIAL;
    static const std::string JSON_CAMERA;
    static const std::string JSON_AUDIO;

public:
    SwitchSystemOption(
        PABotBaseLevel min_pabotbase,
        bool allow_commands_while_running
    );
    SwitchSystemOption(
        PABotBaseLevel min_pabotbase,
        bool allow_commands_while_running,
        const JsonValue& json
    );
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    const bool m_allow_commands_while_running;

    SerialPortOption m_serial;
    CameraOption m_camera;
    AudioOption m_audio;
};





}
}
#endif
