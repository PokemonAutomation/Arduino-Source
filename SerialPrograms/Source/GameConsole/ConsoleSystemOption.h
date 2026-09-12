/*  Console System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a console.
 *  This class maintains no runtime state or UI and is not thread-safe.
 *
 */

#ifndef PokemonAutomation_GameConsole_ConsoleSystemOption_H
#define PokemonAutomation_GameConsole_ConsoleSystemOption_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/VideoOverlayOption.h"
#include "Controllers/ControllerOption.h"

namespace PokemonAutomation{
namespace GameConsole{




class ConsoleSystemOption{
    static const std::string JSON_CAMERA;
    static const std::string JSON_VIDEO;
    static const std::string JSON_AUDIO;
    static const std::string JSON_OVERLAY;
    static const std::string JSON_CONTROLLER;
    static const std::string JSON_CONTROLLERS;

public:
    virtual ~ConsoleSystemOption() = default;
    ConsoleSystemOption(size_t num_controllers);
    ConsoleSystemOption(size_t num_controllers, const JsonValue& json);

    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);


public:
    VideoSourceOption m_video;
    AudioOption m_audio;
    VideoOverlayOption m_overlay;
    FixedLimitVector<ControllerOption> m_controllers;
};




}
}
#endif
