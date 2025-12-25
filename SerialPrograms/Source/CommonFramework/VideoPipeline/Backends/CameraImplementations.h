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


// Abstract base class for camera backend implementation.
// Call `get_camera_backend()` to get the camera backend singleton.
// Current camera backend is implemented in CameraWidgetQt6.h/cpp.
class CameraBackend{
public:
    virtual ~CameraBackend() = default;

    // Get camera backend reference and get all cameras' info.
    // Note: to avoid freezing the UI while launching the application, the camera backend when
    // constructed, use a separate thread to query and load camera info. If you call
    // `get_all_cameras()` immidiately after the backend is constructed, it may not give you
    // all cameras' info.
    virtual std::vector<CameraInfo> get_all_cameras() const = 0;
    virtual std::string get_camera_name(const CameraInfo& info) const = 0;

    virtual std::unique_ptr<VideoSource> make_video_source(
        Logger& logger,
        const CameraInfo& info,
        Resolution resolution
    ) const = 0;
};


// Get the camera backend singleton.
// Current camera backend is implemented in CameraWidgetQt6.h/cpp.
const CameraBackend& get_camera_backend();

// Call `get_camera_backend()` to get camera backend reference and get all cameras' info.
// Note: to avoid freezing the UI while launching the application, the camera backend when
// constructed, use a separate thread to query and load camera info. If you call
// `get_all_cameras()` immidiately after the backend is constructed, it may not give you
// all cameras' info.
std::vector<CameraInfo> get_all_cameras();
std::string get_camera_name(const CameraInfo& info);






}
#endif
