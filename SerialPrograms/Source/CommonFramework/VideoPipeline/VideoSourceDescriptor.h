/*  Video Source Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSourceDescriptor_H
#define PokemonAutomation_VideoPipeline_VideoSourceDescriptor_H

#include <memory>
#include "Common/Cpp/ImageResolution.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "VideoFormats.h"

namespace PokemonAutomation{

class VideoSource;



enum class VideoSourceType{
    None,
    StillImage,
    VideoPlayback,
    Camera,
};



//  - Describe basic attributes of a video source. The source can be webcam or
//    the capture card that streams a Switch console.
//  - Can load and save the attributes to JSON.
//  - Can call make_VideoSource() to create a VideoSource object that manages
//    the video source and can create UI widget to display it.
//  - After user selects a new video source on the UI, call run_post_select()
//    to do some optional additional stuff after the user's selection, e.g.
//    open a file selection dialog box to continue user interaction.
class VideoSourceDescriptor{
public:
    const VideoSourceType type;

public:
    VideoSourceDescriptor(VideoSourceType p_type)
        : type(p_type)
    {}
    virtual ~VideoSourceDescriptor() = default;

public:
    virtual bool should_reload() const{ return false; }
    virtual bool operator==(const VideoSourceDescriptor& x) const = 0;
    virtual std::string display_name() const = 0;

public:
    //  After user selects a new video source on the UI, call this function to
    //  do some optional additional stuff, e.g. open a file selection dialog
    //  box to continue user interaction.
    virtual void run_post_select(){};
    virtual JsonValue to_json() const = 0;
    virtual void load_json(const JsonValue& json) = 0;

public:
    virtual std::unique_ptr<VideoSource> make_VideoSource(
        Logger& logger,
        Resolution resolution,
        VideoFormat format,
        FramesPerSecond fps
    ) const = 0;
};






}
#endif
