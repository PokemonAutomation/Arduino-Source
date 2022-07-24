/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <memory>
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{


struct VideoSnapshot{
    //  The frame itself. Null means no snapshot was available.
    std::shared_ptr<const ImageRGB32> frame;

    //  The timestamp of when the frame was taken.
    //  This will be as close as possible to when the frame was taken.
    WallClock timestamp = WallClock::min();

    VideoSnapshot()
         : frame(std::make_shared<const ImageRGB32>())
         , timestamp(WallClock::min())
    {}
    VideoSnapshot(ImageRGB32 p_frame, WallClock p_timestamp)
         : frame(std::make_shared<const ImageRGB32>(std::move(p_frame)))
         , timestamp(p_timestamp)
    {}

    //  Returns true if the snapshot is valid.
    operator bool() const{ return frame && *frame; }

    operator std::shared_ptr<const ImageRGB32>() &&{ return std::move(frame); }
    operator ImageViewRGB32() const{ return *frame; }
};




//  Define basic interface of a video feed to be used
//  by programs.
class VideoFeed{
public:
    //  Can call from anywhere.
    virtual void async_reset_video() = 0;

    //  Do not call this on the main thread or it may deadlock.
    virtual VideoSnapshot snapshot() = 0;
};







}
#endif
