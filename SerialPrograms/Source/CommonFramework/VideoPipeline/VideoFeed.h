/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/
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
    operator ImageViewRGB32() const{ return *frame; }

    void clear(){
        frame.reset();
        timestamp = WallClock::min();
    }
};


//  Experimental:
//  This video frame callback isn't guaranteed to be supported by all
//  implementations. Unsupported implementations will never fire this callback.
class VideoFrame;
struct VideoFrameListener{
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) = 0;
};


//  Define basic interface of a video feed to be used by programs.
class VideoFeed{
public:
    virtual void add_frame_listener(VideoFrameListener& listener) = 0;
    virtual void remove_frame_listener(VideoFrameListener& listener) = 0;

    //  Reset the video. Note that this may return early.
    virtual void reset() = 0;


public:
    //
    //  Returns a snapshot of the latest available frame.
    //
    //  If the latest frame is not yet ready (not converted to ImageRGB32),
    //  this function will block until it is ready.
    //
    //  If this returns a null snapshot, it means the video isn't available.
    //
    //  Implementations must be able to handle calls to this at high rate from
    //  many different threads simultaneously. So it should perform caching.
    //
    VideoSnapshot snapshot(){
        return snapshot_latest_blocking();
    }
    virtual VideoSnapshot snapshot_latest_blocking() = 0;

    //
    //  This is a non-blocking snapshot function. It will never block, but it
    //  is not guaranteed to return the absolute latest snapshot.
    //
    //  If this returns a null snapshot, it can mean either that the video
    //  isn't available or that no snapshot (>= min_time) is ready.
    //
    //  Implementations must be able to handle calls to this at high rate from
    //  many different threads simultaneously. So it should perform caching.
    //
    //  This function is intended to be called many times in a series. So calls
    //  will be taken as a hint for future calls. Thus the first call to this
    //  function will begin prefetch conversions of frames so they are ready
    //  on future calls.
    //
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) = 0;


public:
    //  Returns the currently measured frames/second for the video source.
    //  Use this for diagnostic purposes.
    virtual double fps_source() const = 0;
    //  Returns the currently measured frames/second for the video display thread.
    //  Use this for diagnostic purposes.
    virtual double fps_display() const = 0;
};







}
#endif
