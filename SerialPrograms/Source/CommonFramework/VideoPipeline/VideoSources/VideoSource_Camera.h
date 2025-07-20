/*  Video Source (Camera)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSource_Camera_H
#define PokemonAutomation_VideoPipeline_VideoSource_Camera_H

#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"

namespace PokemonAutomation{


class VideoSourceDescriptor_Camera : public VideoSourceDescriptor{
public:
    VideoSourceDescriptor_Camera()
        : VideoSourceDescriptor(VideoSourceType::Camera)
    {}
    VideoSourceDescriptor_Camera(CameraInfo info)
        : VideoSourceDescriptor(VideoSourceType::Camera)
        , m_info(std::move(info))
    {}


public:
    virtual bool operator==(const VideoSourceDescriptor& x) const override;
    virtual std::string display_name() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<VideoSource> make_VideoSource(
        Logger& logger,
        Resolution resolution
    ) const override;


private:
    CameraInfo m_info;
};



}
#endif
