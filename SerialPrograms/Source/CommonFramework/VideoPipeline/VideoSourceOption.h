/*  Video Source Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSourceOption_H
#define PokemonAutomation_VideoPipeline_VideoSourceOption_H

#include <map>
#include "VideoSourceDescriptor.h"

namespace PokemonAutomation{


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

    //  Use a cache to help create a VideoSoruceDescriptor.
    //  The cache remembers the last used descriptor for each interface type. That way when
    //  the user switches back-and-forth between two interfaces, it will reload the previous
    //  one and return faster.
    //  If not found in cache, it will create a new one according to the type.
    void set_descriptor(std::shared_ptr<VideoSourceDescriptor> descriptor);
    std::shared_ptr<VideoSourceDescriptor> get_descriptor_from_cache(VideoSourceType type);


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    Resolution m_resolution;
    VideoFormat m_format;
    FramesPerSecond m_fps;

private:
    std::shared_ptr<VideoSourceDescriptor> m_descriptor;
    std::map<VideoSourceType, std::shared_ptr<VideoSourceDescriptor>> m_descriptor_cache;
};


}
#endif
