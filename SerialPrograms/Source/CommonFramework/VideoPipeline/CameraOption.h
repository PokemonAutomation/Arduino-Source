/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraOption_H
#define PokemonAutomation_VideoPipeline_CameraOption_H

#include "Common/Cpp/ImageResolution.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CameraInfo.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class VideoDisplayWidget;


class CameraSelectorWidget;

//  Handles the state of video selection: video source and resolution.
//  Call make_ui() to generate the UI friend class CameraSelectorWidget,
//  which directly modifies CameraOption's internal state.
//  This separates state from UI.
class CameraOption{
    static const std::string JSON_CAMERA;
    static const std::string JSON_RESOLUTION;

public:
    CameraOption(Resolution default_resolution);
    CameraOption(Resolution default_resolution, const JsonValue& json);

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    CameraSelectorWidget* make_ui(QWidget& parent, LoggerQt& logger, VideoDisplayWidget& holder);

private:
    friend class CameraSession;
    friend class CameraSelectorWidget;

    const Resolution m_default_resolution;
    CameraInfo m_info;
    Resolution m_current_resolution;
};





}
#endif
