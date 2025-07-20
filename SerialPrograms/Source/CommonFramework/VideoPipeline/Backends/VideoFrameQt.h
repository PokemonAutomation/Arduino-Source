/*  Video Frame
 *
 *  From: https://github.com/PokemonAutomation/
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

    VideoFrame(VideoFrame&&) = default;
    VideoFrame& operator=(VideoFrame&&) = default;
    VideoFrame(const VideoFrame&) = delete;
    void operator=(const VideoFrame&) = delete;

    VideoFrame()
        : timestamp(WallClock::min())
    {}
    VideoFrame(WallClock p_timestamp, QVideoFrame p_frame)
        : timestamp(p_timestamp)
        , frame(std::move(p_frame))
    {}

    void clear(){
        frame = QVideoFrame();
    }
    bool is_valid() const{
        return frame.isValid();
    }
};


}
#endif
