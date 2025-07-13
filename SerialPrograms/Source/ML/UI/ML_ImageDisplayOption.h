/*  ML Image Display Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of an image display and label UI.
 * 
 *  This class is not thread-safe. To access internal state of an image display session
 *  in a thread-safe way, use ImageDisplaySession.
 *
 */

#ifndef ML_ImageDisplayOption_H
#define ML_ImageDisplayOption_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/VideoOverlayOption.h"
#include "Controllers/ControllerCapability.h"
#include "Controllers/ControllerDescriptor.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
    class ControllerRequirements;
namespace ML{


// options to load an image for image labeling purposes
class ImageDisplayOption{
    static const std::string JSON_CONTROLLER;
    static const std::string JSON_CAMERA;
    static const std::string JSON_VIDEO;
    static const std::string JSON_AUDIO;
    static const std::string JSON_OVERLAY;
    static const std::string JSON_CONSOLE_TYPE;

public:
    ImageDisplayOption();
    ImageDisplayOption(const JsonValue& json);

    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    ControllerOption m_controller;
    VideoSourceOption m_video;
    AudioOption m_audio;
    VideoOverlayOption m_overlay;
};





}
}
#endif
