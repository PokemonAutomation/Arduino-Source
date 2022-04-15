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



}
#endif
