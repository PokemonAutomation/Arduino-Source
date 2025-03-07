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
#include "CommonFramework/VideoPipeline/CameraOption.h"
#include "CommonFramework/VideoPipeline/VideoOverlayOption.h"
#include "Controllers/ControllerCapability.h"
#include "Controllers/ControllerDescriptor.h"

namespace PokemonAutomation{
    class ControllerRequirements;
namespace NintendoSwitch{


//Color pick_color(FeedbackType feedback);
Color pick_color(
    const ControllerFeatures& required_features,
    FasterIfTickPrecise faster_if_tick_precise
);


class SwitchSystemOption{
    static const std::string JSON_CONTROLLER;
    static const std::string JSON_CAMERA;
    static const std::string JSON_AUDIO;
    static const std::string JSON_OVERLAY;

public:
    SwitchSystemOption(
        const ControllerFeatures& required_features,
        bool allow_commands_while_running
    );
    SwitchSystemOption(
        const ControllerFeatures& required_features,
        bool allow_commands_while_running,
        const JsonValue& json
    );

    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    const ControllerFeatures& m_required_features;
    const bool m_allow_commands_while_running;

    ControllerOption m_controller;
    CameraOption m_camera;
    AudioOption m_audio;
    VideoOverlayOption m_overlay;
};





}
}
#endif
