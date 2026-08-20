/*  Format Aggregator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_QFormatAggregator_H
#define PokemonAutomation_VideoPipeline_QFormatAggregator_H

#include <QCameraFormat>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/ImageResolution.h"
#include "CommonFramework/VideoPipeline/VideoFormats.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"

namespace PokemonAutomation{



void get_format(
    const QCameraFormat& qformat,
    Resolution& resolution,
    VideoFormat& format,
    FramesPerSecond& fps
);

QCameraFormat build_format_set(
    Logger& logger,
    VideoFormatSet& format_set,
    const QCameraDevice& device,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
);



}
#endif
