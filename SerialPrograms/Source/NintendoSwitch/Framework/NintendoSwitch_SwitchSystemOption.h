/*  Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/
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
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/VideoOverlayOption.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "Controllers/ControllerDescriptor.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
    class ControllerRequirements;
namespace NintendoSwitch{


Color pick_color(ProgramControllerClass color_class);


// options to control and monitor a Switch. It inlcudes
// what micro-controller and what video source to use and
// what video overlay display option to set.
class SwitchSystemOption{
    static const std::string JSON_CONTROLLER;
    static const std::string JSON_CAMERA;
    static const std::string JSON_VIDEO;
    static const std::string JSON_AUDIO;
    static const std::string JSON_OVERLAY;
    static const std::string JSON_CONSOLE_TYPE;

public:
    SwitchSystemOption(
        bool allow_commands_while_running
    );
    SwitchSystemOption(
        bool allow_commands_while_running,
        const JsonValue& json
    );

    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    const bool m_allow_commands_while_running;

    ControllerOption m_controller;
    VideoSourceOption m_video;
    AudioOption m_audio;
    VideoOverlayOption m_overlay;
    ConsoleModelCell m_console_type;
};





}
}
#endif
