/*  Video Source Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSourceDescriptor_H
#define PokemonAutomation_VideoPipeline_VideoSourceDescriptor_H

#include <memory>
#include <map>
#include "Common/Cpp/ImageResolution.h"
#include "Common/Cpp/Json/JsonValue.h"

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
    virtual void load_json(const JsonValue& json) = 0;
    virtual JsonValue to_json() const = 0;

public:
    virtual std::unique_ptr<VideoSource> make_VideoSource(
        Logger& logger,
        Resolution resolution
    ) const = 0;
};



//  VideoSourceOption manages VideoSourceDescriptor so user can switch between different
//  video sources. User can set new VideoSourceDescriptor to it. It also stores the current
//  video resolution.
//  It has a cache to store last used VideoSourceDescriptor for faster video source switch.
//  It is used as the input to build a VideoSession.
class VideoSourceOption{
public:
    VideoSourceOption();

    std::shared_ptr<VideoSourceDescriptor> descriptor() const{
        return m_descriptor;
    }
    void set_descriptor(std::shared_ptr<VideoSourceDescriptor> descriptor);

    //  Use a cache to help create a VideoSoruceDescriptor.
    //  The cache remembers the last used descriptor for each interface type. That way when
    //  the user switches back-and-forth between two interfaces, it will reload the previous
    //  one and return faster.
    //  If not found in cache, it will create a new one according to the type.
    std::shared_ptr<VideoSourceDescriptor> get_descriptor_from_cache(VideoSourceType type);

public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;

public:
    Resolution m_resolution;

private:
    std::shared_ptr<VideoSourceDescriptor> m_descriptor;
    std::map<VideoSourceType, std::shared_ptr<VideoSourceDescriptor>> m_descriptor_cache;
};




}
#endif
