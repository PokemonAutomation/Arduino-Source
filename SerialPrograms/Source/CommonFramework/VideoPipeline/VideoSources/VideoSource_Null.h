/*  Video Source (Null)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSource_Null_H
#define PokemonAutomation_VideoPipeline_VideoSource_Null_H

#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"

namespace PokemonAutomation{


class VideoSourceDescriptor_Null : public VideoSourceDescriptor{
public:
    VideoSourceDescriptor_Null()
        : VideoSourceDescriptor(VideoSourceType::None)
    {}

    virtual bool operator==(const VideoSourceDescriptor& x) const override;
    virtual std::string display_name() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<VideoSource> make_VideoSource(
        Logger& logger,
        Resolution resolution
    ) const override;
};



}
#endif
