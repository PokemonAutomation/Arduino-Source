/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraImplementations_H
#define PokemonAutomation_VideoPipeline_CameraImplementations_H

#include <functional>
#include <vector>
#include <QString>
#include "CommonFramework/Logging/Logger.h"
#include "CameraInfo.h"
#include "VideoWidget.h"

namespace PokemonAutomation{


std::vector<CameraInfo> get_all_cameras();
QString get_camera_name(const CameraInfo& info);

std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
);


}
#endif
