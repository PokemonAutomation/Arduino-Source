/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraImplementations_H
#define PokemonAutomation_VideoPipeline_CameraImplementations_H

#include <vector>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoSourceDescriptor.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"

namespace PokemonAutomation{


class VideoBackendOption : public IntegerEnumDropdownOption{
public:
    VideoBackendOption();
};



class CameraBackend{
public:
    virtual ~CameraBackend() = default;

    virtual std::vector<CameraInfo> get_all_cameras() const = 0;
    virtual std::string get_camera_name(const CameraInfo& info) const = 0;

    virtual std::unique_ptr<VideoSource> make_video_source(
        Logger& logger,
        const CameraInfo& info,
        Resolution resolution
    ) const = 0;
};


const CameraBackend& get_camera_backend();

std::vector<CameraInfo> get_all_cameras();
std::string get_camera_name(const CameraInfo& info);






}
#endif
