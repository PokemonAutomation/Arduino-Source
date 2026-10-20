/*  Video Frame
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoFrame_H
#define PokemonAutomation_VideoFrame_H

#include <QVideoFrame>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoFormats.h"

namespace PokemonAutomation{


VideoFormat QVideoFrameFormat_to_VideoFormat(QVideoFrameFormat::PixelFormat format);
QVideoFrameFormat::PixelFormat VideoFormat_to_QVideoFrameFormat(VideoFormat format);



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


QVideoFrame deep_copy_frame(const QVideoFrame& frame);

void report_video_path_stats(
    Logger& logger, const char* backend,
    const QVideoFrame& src, bool copy_ok,
    WallClock copy_start, WallClock copy_end
);

}
#endif
