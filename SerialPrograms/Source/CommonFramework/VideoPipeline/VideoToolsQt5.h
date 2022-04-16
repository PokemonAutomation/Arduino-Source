/*  Video Tools (QT5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoToolsQt5_H
#define PokemonAutomation_VideoPipeline_VideoToolsQt5_H

#include <QImage>
#include <QVideoFrame>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{


QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical);

bool determine_frame_orientation(
    Logger& logger, const QImage& reference, const QImage& image,
    bool& flip_vertical
);



}
#endif
