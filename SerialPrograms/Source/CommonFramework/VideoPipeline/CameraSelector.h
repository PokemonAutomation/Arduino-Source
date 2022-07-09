/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraSelector_H
#define PokemonAutomation_VideoPipeline_CameraSelector_H

#include <QSize>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CameraInfo.h"

class QWidget;

namespace PokemonAutomation{

class JsonValue;
class VideoDisplayWidget;


class CameraSelectorWidget;

// Handles the state of video selection: video source and resolution.
// Call make_ui() to generate the UI friend class CameraSelectorWidget,
// which directly modifies CameraSelector's internal state.
// This separates state from UI.
class CameraSelector{
    static const std::string JSON_CAMERA;
    static const std::string JSON_RESOLUTION;

public:
    CameraSelector(QSize default_resolution);
    CameraSelector(QSize default_resolution, const JsonValue& json);

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    CameraSelectorWidget* make_ui(QWidget& parent, LoggerQt& logger, VideoDisplayWidget& holder);

private:
    friend class CameraSelectorWidget;
    const QSize m_default_resolution;
    CameraInfo m_camera;
    QSize m_current_resolution;
};





}
#endif
