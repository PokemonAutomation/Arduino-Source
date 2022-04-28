/*  Video Tools (QT5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoToolsQt5_H
#define PokemonAutomation_VideoPipeline_VideoToolsQt5_H

#include <vector>
#include <QImage>
#include <QVideoFrame>
#include "Common/Cpp/AbstractLogger.h"
#include "CameraInfo.h"

namespace PokemonAutomation{


std::vector<CameraInfo> qt5_get_all_cameras();
QString qt5_get_camera_name(const CameraInfo& info);




QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical);

bool determine_frame_orientation(
    Logger& logger, const QImage& reference, const QImage& image,
    bool& flip_vertical
);



}
#endif
