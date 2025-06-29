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
    virtual void run_post_select(){};
    virtual void load_json(const JsonValue& json) = 0;
    virtual JsonValue to_json() const = 0;

public:
    virtual std::unique_ptr<VideoSource> make_VideoSource(
        Logger& logger,
        Resolution resolution
    ) const = 0;
};




class VideoSourceOption{
public:
    VideoSourceOption();

    std::shared_ptr<VideoSourceDescriptor> descriptor() const{
        return m_descriptor;
    }
    void set_descriptor(std::shared_ptr<VideoSourceDescriptor> descriptor);

    //  Remember the last used descriptor for each interface type. That way when
    //  the user switches back-and-forth between two interfaces, it will reload
    //  the previous one.
    std::shared_ptr<VideoSourceDescriptor> get_descriptor_from_cache(VideoSourceType type);


public:
    void load_json(const JsonValue& json);
    JsonValue to_json() const;


private:
    std::shared_ptr<VideoSourceDescriptor> m_descriptor;
public:
    Resolution m_resolution;

private:
    std::map<VideoSourceType, std::shared_ptr<VideoSourceDescriptor>> m_descriptor_cache;
};




}
#endif
