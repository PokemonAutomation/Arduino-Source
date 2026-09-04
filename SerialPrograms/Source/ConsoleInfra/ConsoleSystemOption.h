/*  Console System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a console.
 *  This class maintains no runtime state or UI and is not thread-safe.
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_ConsoleSystemOption_H
#define PokemonAutomation_ConsoleInfra_ConsoleSystemOption_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/VideoOverlayOption.h"
#include "Controllers/ControllerOption.h"

namespace PokemonAutomation{
namespace ConsoleInfra{




class ConsoleSystemOption{
    static const std::string JSON_CAMERA;
    static const std::string JSON_VIDEO;
    static const std::string JSON_AUDIO;
    static const std::string JSON_OVERLAY;
    static const std::string JSON_CONTROLLER;
    static const std::string JSON_CONTROLLERS;

public:
    virtual ~ConsoleSystemOption() = default;
    ConsoleSystemOption(
        size_t num_controllers,
        bool allow_commands_while_locked
    );
    ConsoleSystemOption(
        size_t num_controllers,
        bool allow_commands_while_locked,
        const JsonValue& json
    );

    virtual void load_json(const JsonValue& json);
    virtual JsonValue to_json() const;


public:
    const bool m_allow_commands_while_locked;

    VideoSourceOption m_video;
    AudioOption m_audio;
    VideoOverlayOption m_overlay;
    FixedLimitVector<ControllerOption> m_controllers;
};




}
}
#endif
