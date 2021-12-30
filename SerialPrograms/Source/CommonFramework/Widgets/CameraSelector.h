/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CameraSelector_H
#define PokemonAutomation_CameraSelector_H

#include <QSize>
#include "CommonFramework/Logging/Logger.h"
#include "CameraInfo.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{

class CameraInfo;
class VideoDisplayWidget;


class CameraSelectorWidget;


class CameraSelector{
    static const QString JSON_CAMERA;
    static const QString JSON_RESOLUTION;

public:
    CameraSelector();
    CameraSelector(const QJsonValue& json);

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    CameraSelectorWidget* make_ui(QWidget& parent, Logger& logger, VideoDisplayWidget& holder);

private:
    friend class CameraSelectorWidget;
    CameraInfo m_camera;
    QSize m_resolution;
};





}
#endif
