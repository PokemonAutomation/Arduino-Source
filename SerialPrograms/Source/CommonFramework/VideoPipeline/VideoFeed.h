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
    explicit operator bool() const{ return frame && *frame; }

    const ImageRGB32* operator->() const{ return frame.get(); }

    operator std::shared_ptr<const ImageRGB32>() const{ return frame; }
    operator ImageViewRGB32() const&{ return *frame; }
    operator ImageViewRGB32() && = delete;

    void clear(){
        frame.reset();
        timestamp = WallClock::min();
    }
};




//  Define basic interface of a video feed to be used
//  by programs.
class VideoFeed{
public:
    //  Reset the video. Note that this may return early.
    virtual void reset() = 0;

    //  Do not call this on the main thread or it may deadlock.
    virtual VideoSnapshot snapshot() = 0;

    //  Returns the currently measured frames/second for the video source + display.
    //  Use this for diagnostic purposes.
    virtual double fps_source() = 0;
    virtual double fps_display() = 0;
};







}
#endif
