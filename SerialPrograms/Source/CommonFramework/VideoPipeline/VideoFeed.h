/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <memory>
#include <QImage>
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{


struct VideoSnapshot{
    //  The frame itself. Null means no snapshot was available.
    QImage frame;
    std::shared_ptr<const ImageRGB32> frame_ptr;

    //  The timestamp of when the frame was taken.
    //  This will be as close as possible to when the frame was taken.
    WallClock timestamp = WallClock::min();

    VideoSnapshot(QImage p_frame = QImage(), WallClock p_timestamp = WallClock::min())
         : frame(std::move(p_frame))
         , frame_ptr(std::make_shared<const ImageRGB32>(frame))
         , timestamp(p_timestamp)
    {}

    operator bool() const{ return !frame.isNull(); }

//    operator const QImage&() const{ return frame; }

    operator std::shared_ptr<const ImageRGB32>() &&{ return std::move(frame_ptr); }
    operator ImageViewRGB32() const{ return *frame_ptr; }
};




//  Define basic interface of a video feed to be used
//  by programs.
class VideoFeed{
public:
    //  Can call from anywhere.
    virtual void async_reset_video() = 0;

    //  Do not call this on the main thread or it will deadlock.
    //  Format must be "Format_ARGB32" or "Format_RGB32".
    virtual VideoSnapshot snapshot() = 0;
};







}
#endif
