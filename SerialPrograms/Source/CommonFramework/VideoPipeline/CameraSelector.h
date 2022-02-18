/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraSelector_H
#define PokemonAutomation_VideoPipeline_CameraSelector_H

#include <QSize>
#include "CommonFramework/Logging/Logger.h"
#include "CameraInfo.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{

class VideoDisplayWidget;


class CameraSelectorWidget;

// Handles the state of video selection: video source and resolution.
// Call make_ui() to generate the UI friend class CameraSelectorWidget,
// which directly modifies CameraSelector's internal state.
// This separates state from UI.
class CameraSelector{
    static const QString JSON_CAMERA;
    static const QString JSON_RESOLUTION;

public:
    CameraSelector(QSize default_resolution);
    CameraSelector(QSize default_resolution, const QJsonValue& json);

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    CameraSelectorWidget* make_ui(QWidget& parent, Logger& logger, VideoDisplayWidget& holder);

private:
    friend class CameraSelectorWidget;
    const QSize m_default_resolution;
    CameraInfo m_camera;
    QSize m_current_resolution;
};





}
#endif
