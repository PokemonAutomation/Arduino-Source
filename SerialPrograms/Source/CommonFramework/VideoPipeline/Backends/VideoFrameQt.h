/*  Video Frame
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFrame_H
#define PokemonAutomation_VideoFrame_H

#include <QVideoFrame>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


class VideoFrame{
public:
    WallClock timestamp;
    QVideoFrame frame;

    VideoFrame(const VideoFrame&) = delete;
    void operator=(const VideoFrame&) = delete;

    VideoFrame(WallClock p_timestamp, QVideoFrame p_frame)
        : timestamp(p_timestamp)
        , frame(std::move(p_frame))
    {}
};


}
#endif
